
/*
 * netst.cpp - network socket status utility
 *
 * Linux:
 *   Socket enumeration: NETLINK_SOCK_DIAG
 *   PID association:    /proc/<pid>/fd (process inspection only)
 *
 * Windows:
 *   Socket enumeration and PID: IP Helper API
 *   Process path:               QueryFullProcessImageNameW
 *
 * FreeBSD:
 *   Process/socket inspection: libprocstat
 *
 * Build:
 *   Linux:    g++ -std=c++17 -O2 -Wall -Wextra netst.cpp -o netst
 *   FreeBSD:  c++ -std=c++17 -O2 netst.cpp -o netst -lprocstat
 *   Windows:  g++ -std=c++17 -O2 netst.cpp -o netst.exe -liphlpapi -lws2_32
 *
 * OpenBSD and NetBSD backends are not implemented yet.
 */

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <tlhelp32.h>

#elif defined(__linux__)

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/netlink.h>
#include <linux/inet_diag.h>
#include <linux/sock_diag.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#elif defined(__FreeBSD__)

#include <sys/types.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <libprocstat.h>

static std::string endpoint(
    const sockaddr* sa,
    socklen_t len
) {
    char host[NI_MAXHOST] = {};
    char serv[NI_MAXSERV] = {};

    if (!sa || !sa->sa_family)
        return "*:*";

    if (getnameinfo(
            sa, len,
            host, sizeof(host),
            serv, sizeof(serv),
            NI_NUMERICHOST | NI_NUMERICSERV
        ) != 0) {
        return "*:*";
    }

    return std::string(host) + ":" + serv;
}

#else
#error "netst: this operating system backend is not implemented yet"
#endif

struct NetSocket {
    std::string local;
    std::string remote;
    std::string protocol;
    std::string state;

    int pid = -1;
    std::string process_path;
    std::string process_args;

    uint64_t socket_id = 0;
};

static std::vector<NetSocket> sockets;

/*
 * Escape a field for pipe-delimited output.
 */
static std::string escape_field(const std::string& value) {
    std::string result;

    for (char c : value) {
        switch (c) {
            case '\\': result += "\\\\"; break;
            case '|':  result += "\\|";  break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;     break;
        }
    }

    return result;
}

#ifdef __linux__

static std::string linux_address(
    const inet_diag_msg& msg,
    bool local
) {
    char address[INET6_ADDRSTRLEN] = {};

    const void* src = local
        ? static_cast<const void*>(msg.id.idiag_src)
        : static_cast<const void*>(msg.id.idiag_dst);

    if (msg.idiag_family == AF_INET) {
        inet_ntop(AF_INET, src, address, sizeof(address));
    } else {
        inet_ntop(AF_INET6, src, address, sizeof(address));
    }

    uint16_t port = ntohs(
        local ? msg.id.idiag_sport : msg.id.idiag_dport
    );

    return std::string(address) + ":" + std::to_string(port);
}

static std::string linux_tcp_state(unsigned char state) {
    switch (state) {
        case TCP_ESTABLISHED: return "ESTABLISHED";
        case TCP_SYN_SENT:    return "SYN_SENT";
        case TCP_SYN_RECV:    return "SYN_RECV";
        case TCP_FIN_WAIT1:   return "FIN_WAIT1";
        case TCP_FIN_WAIT2:   return "FIN_WAIT2";
        case TCP_TIME_WAIT:   return "TIME_WAIT";
        case TCP_CLOSE:       return "CLOSE";
        case TCP_CLOSE_WAIT:  return "CLOSE_WAIT";
        case TCP_LAST_ACK:    return "LAST_ACK";
        case TCP_LISTEN:      return "LISTEN";
        case TCP_CLOSING:     return "CLOSING";
        default:              return "UNKNOWN";
    }
}

static bool linux_diag_dump(
    int fd,
    int family,
    int protocol,
    std::vector<NetSocket>& out
) {
    struct {
        nlmsghdr header;
        inet_diag_req_v2 request;
    } req{};

    req.header.nlmsg_len = sizeof(req);
    req.header.nlmsg_type = SOCK_DIAG_BY_FAMILY;
    req.header.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.header.nlmsg_seq = 1;

    req.request.sdiag_family = family;
    req.request.sdiag_protocol = protocol;
    req.request.idiag_states = 0xFFFFFFFF;

    sockaddr_nl kernel{};
    kernel.nl_family = AF_NETLINK;

    if (sendto(
            fd,
            &req,
            sizeof(req),
            0,
            reinterpret_cast<sockaddr*>(&kernel),
            sizeof(kernel)
        ) < 0) {
        return false;
    }

    char buffer[65536];

    while (true) {
        ssize_t received = recv(fd, buffer, sizeof(buffer), 0);

        if (received <= 0)
            return false;

        for (
            nlmsghdr* hdr = reinterpret_cast<nlmsghdr*>(buffer);
            NLMSG_OK(hdr, static_cast<unsigned int>(received));
            hdr = NLMSG_NEXT(hdr, received)
        ) {
            if (hdr->nlmsg_type == NLMSG_DONE)
                return true;

            if (hdr->nlmsg_type == NLMSG_ERROR) {
                auto* err = reinterpret_cast<nlmsgerr*>(
                    NLMSG_DATA(hdr)
                );

                if (err->error != 0)
                    return false;

                continue;
            }

            if (hdr->nlmsg_type != SOCK_DIAG_BY_FAMILY)
                continue;

            auto* msg = reinterpret_cast<inet_diag_msg*>(
                NLMSG_DATA(hdr)
            );

            NetSocket s;

            s.local = linux_address(*msg, true);
            s.remote = linux_address(*msg, false);
            s.protocol = protocol == IPPROTO_TCP ? "TCP" : "UDP";
            s.state = protocol == IPPROTO_TCP
                ? linux_tcp_state(msg->idiag_state)
                : "UNCONN";

            s.socket_id = msg->idiag_inode;

            out.push_back(std::move(s));
        }
    }
}

static std::string read_link(const std::string& path) {
    char buf[4096];

    ssize_t n = readlink(path.c_str(), buf, sizeof(buf) - 1);

    if (n < 0)
        return {};

    buf[n] = '\0';
    return buf;
}

static std::string read_cmdline(const std::string& path) {
    int fd = open(path.c_str(), O_RDONLY);

    if (fd < 0)
        return {};

    char buf[16384];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n <= 0)
        return {};

    buf[n] = '\0';

    for (ssize_t i = 0; i < n; ++i) {
        if (buf[i] == '\0')
            buf[i] = ' ';
    }

    std::string result(buf);

    while (!result.empty() && result.back() == ' ')
        result.pop_back();

    return result;
}

static void linux_find_processes(std::vector<NetSocket>& out) {
    DIR* proc = opendir("/proc");

    if (!proc)
        return;

    dirent* entry;

    while ((entry = readdir(proc))) {
        char* end = nullptr;
        long pid = strtol(entry->d_name, &end, 10);

        if (!end || *end || pid <= 0)
            continue;

        std::string fd_path =
            "/proc/" + std::to_string(pid) + "/fd";

        DIR* fds = opendir(fd_path.c_str());

        if (!fds)
            continue;

        std::string exe = read_link(
            "/proc/" + std::to_string(pid) + "/exe"
        );

        std::string args = read_cmdline(
            "/proc/" + std::to_string(pid) + "/cmdline"
        );

        dirent* fd_entry;

        while ((fd_entry = readdir(fds))) {
            if (fd_entry->d_name[0] == '.')
                continue;

            std::string link = read_link(
                fd_path + "/" + fd_entry->d_name
            );

            if (link.rfind("socket:[", 0) != 0)
                continue;

            size_t end_pos = link.find(']');

            if (end_pos == std::string::npos)
                continue;

            uint64_t inode = 0;

            try {
                inode = std::stoull(
                    link.substr(8, end_pos - 8)
                );
            } catch (...) {
                continue;
            }

            for (auto& s : out) {
                if (s.socket_id != inode)
                    continue;

                s.pid = static_cast<int>(pid);
                s.process_path = exe;
                s.process_args = args;
            }
        }

        closedir(fds);
    }

    closedir(proc);
}

static bool enumerate_sockets(std::vector<NetSocket>& out) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);

    if (fd < 0) {
        std::cerr << "netst: cannot open NETLINK_SOCK_DIAG\n";
        return false;
    }

    bool ok = true;

    for (int family : {AF_INET, AF_INET6}) {
        ok &= linux_diag_dump(fd, family, IPPROTO_TCP, out);
        ok &= linux_diag_dump(fd, family, IPPROTO_UDP, out);
    }

    close(fd);

    linux_find_processes(out);
    return ok;
}

#elif defined(_WIN32)

static std::string wide_to_utf8(const std::wstring& value) {
    if (value.empty())
        return {};

    int size = WideCharToMultiByte(
        CP_UTF8, 0, value.c_str(), -1,
        nullptr, 0, nullptr, nullptr
    );

    if (size <= 1)
        return {};

    std::string result(size, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0, value.c_str(), -1,
        result.data(), size, nullptr, nullptr
    );

    result.pop_back();
    return result;
}

static std::string windows_process_path(DWORD pid) {
    HANDLE process = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION,
        FALSE,
        pid
    );

    if (!process)
        return {};

    wchar_t path[32768] = {};
    DWORD size = sizeof(path) / sizeof(path[0]);

    std::string result;

    if (QueryFullProcessImageNameW(process, 0, path, &size))
        result = wide_to_utf8(path);

    CloseHandle(process);
    return result;
}

static std::string windows_ipv4(DWORD address, DWORD port) {
    IN_ADDR addr{};
    addr.S_un.S_addr = address;

    char text[INET_ADDRSTRLEN] = {};

    inet_ntop(AF_INET, &addr, text, sizeof(text));

    return std::string(text) + ":" +
        std::to_string(ntohs(static_cast<u_short>(port)));
}

static std::string windows_ipv6(
    const UCHAR* address,
    DWORD port,
    DWORD scope
) {
    IN6_ADDR addr{};
    memcpy(&addr, address, sizeof(addr));

    char text[INET6_ADDRSTRLEN] = {};

    inet_ntop(AF_INET6, &addr, text, sizeof(text));

    std::string result(text);

    if (scope)
        result += "%" + std::to_string(scope);

    return result + ":" +
        std::to_string(ntohs(static_cast<u_short>(port)));
}

static void windows_add_process(
    NetSocket& s,
    DWORD pid
) {
    s.pid = static_cast<int>(pid);
    s.process_path = windows_process_path(pid);

    // IP Helper does not expose process command-line arguments.
    s.process_args = {};
}

static bool windows_tcp_table(
    ADDRESS_FAMILY family,
    std::vector<NetSocket>& out
) {
    ULONG size = 0;

    DWORD status = GetExtendedTcpTable(
        nullptr, &size, FALSE, family,
        TCP_TABLE_OWNER_PID_ALL, 0
    );

    if (status != ERROR_INSUFFICIENT_BUFFER)
        return false;

    std::vector<unsigned char> buffer(size);

    status = GetExtendedTcpTable(
        buffer.data(), &size, FALSE, family,
        TCP_TABLE_OWNER_PID_ALL, 0
    );

    if (status != NO_ERROR)
        return false;

    if (family == AF_INET) {
        auto* table = reinterpret_cast<MIB_TCPTABLE_OWNER_PID*>(
            buffer.data()
        );

        for (DWORD i = 0; i < table->dwNumEntries; ++i) {
            const auto& row = table->table[i];

            NetSocket s;
            s.protocol = "TCP";
            s.local = windows_ipv4(
                row.dwLocalAddr, row.dwLocalPort
            );
            s.remote = windows_ipv4(
                row.dwRemoteAddr, row.dwRemotePort
            );

            switch (row.dwState) {
                case MIB_TCP_STATE_CLOSED:     s.state = "CLOSED"; break;
                case MIB_TCP_STATE_LISTEN:     s.state = "LISTEN"; break;
                case MIB_TCP_STATE_SYN_SENT:   s.state = "SYN_SENT"; break;
                case MIB_TCP_STATE_SYN_RCVD:   s.state = "SYN_RECV"; break;
                case MIB_TCP_STATE_ESTAB:      s.state = "ESTABLISHED"; break;
                case MIB_TCP_STATE_FIN_WAIT1:  s.state = "FIN_WAIT1"; break;
                case MIB_TCP_STATE_FIN_WAIT2:  s.state = "FIN_WAIT2"; break;
                case MIB_TCP_STATE_CLOSE_WAIT: s.state = "CLOSE_WAIT"; break;
                case MIB_TCP_STATE_CLOSING:    s.state = "CLOSING"; break;
                case MIB_TCP_STATE_LAST_ACK:   s.state = "LAST_ACK"; break;
                case MIB_TCP_STATE_TIME_WAIT:  s.state = "TIME_WAIT"; break;
                case MIB_TCP_STATE_DELETE_TCB: s.state = "DELETE_TCB"; break;
                default:                       s.state = "UNKNOWN"; break;
            }

            windows_add_process(s, row.dwOwningPid);
            out.push_back(std::move(s));
        }
    } else {
        auto* table = reinterpret_cast<MIB_TCP6TABLE_OWNER_PID*>(
            buffer.data()
        );

        for (DWORD i = 0; i < table->dwNumEntries; ++i) {
            const auto& row = table->table[i];

            NetSocket s;
            s.protocol = "TCP";
            s.local = windows_ipv6(
                row.ucLocalAddr, row.dwLocalPort,
                row.dwLocalScopeId
            );
            s.remote = windows_ipv6(
                row.ucRemoteAddr, row.dwRemotePort,
                row.dwRemoteScopeId
            );

            s.state = row.State == MIB_TCP_STATE_LISTEN
                ? "LISTEN"
                : row.State == MIB_TCP_STATE_ESTAB
                    ? "ESTABLISHED"
                    : "OTHER";

            windows_add_process(s, row.dwOwningPid);
            out.push_back(std::move(s));
        }
    }

    return true;
}

static bool windows_udp_table(
    ADDRESS_FAMILY family,
    std::vector<NetSocket>& out
) {
    ULONG size = 0;

    DWORD status = GetExtendedUdpTable(
        nullptr, &size, FALSE, family,
        UDP_TABLE_OWNER_PID, 0
    );

    if (status != ERROR_INSUFFICIENT_BUFFER)
        return false;

    std::vector<unsigned char> buffer(size);

    status = GetExtendedUdpTable(
        buffer.data(), &size, FALSE, family,
        UDP_TABLE_OWNER_PID, 0
    );

    if (status != NO_ERROR)
        return false;

    if (family == AF_INET) {
        auto* table = reinterpret_cast<MIB_UDPTABLE_OWNER_PID*>(
            buffer.data()
        );

        for (DWORD i = 0; i < table->dwNumEntries; ++i) {
            const auto& row = table->table[i];

            NetSocket s;
            s.protocol = "UDP";
            s.state = "UNCONN";
            s.local = windows_ipv4(
                row.dwLocalAddr, row.dwLocalPort
            );
            s.remote = "*:*";

            windows_add_process(s, row.dwOwningPid);
            out.push_back(std::move(s));
        }
    } else {
        auto* table = reinterpret_cast<MIB_UDP6TABLE_OWNER_PID*>(
            buffer.data()
        );

        for (DWORD i = 0; i < table->dwNumEntries; ++i) {
            const auto& row = table->table[i];

            NetSocket s;
            s.protocol = "UDP";
            s.state = "UNCONN";
            s.local = windows_ipv6(
                row.ucLocalAddr, row.dwLocalPort,
                row.dwLocalScopeId
            );
            s.remote = "*:*";

            windows_add_process(s, row.dwOwningPid);
            out.push_back(std::move(s));
        }
    }

    return true;
}

static bool enumerate_sockets(std::vector<NetSocket>& out) {
    bool ok = true;

    ok &= windows_tcp_table(AF_INET, out);
    ok &= windows_tcp_table(AF_INET6, out);
    ok &= windows_udp_table(AF_INET, out);
    ok &= windows_udp_table(AF_INET6, out);

    return ok;
}

#elif defined(__FreeBSD__)

static std::string freebsd_address(const sockaddr_storage& sa) {
    return endpoint(
        reinterpret_cast<const sockaddr*>(&sa),
        sa.ss_len
    );
}

static bool enumerate_sockets(std::vector<NetSocket>& out) {
    procstat* ps = procstat_open_sysctl();

    if (!ps) {
        std::cerr << "netst: procstat_open_sysctl failed\n";
        return false;
    }

    unsigned int count = 0;

    kinfo_proc* procs = procstat_getprocs(
        ps, KERN_PROC_PROC, 0, &count
    );

    if (!procs) {
        procstat_close(ps);
        return false;
    }

    for (unsigned int i = 0; i < count; ++i) {
        kinfo_proc* kp = &procs[i];

        filestat_list* files = procstat_getfiles(ps, kp, 0);

        if (!files)
            continue;

        filestat* fs;

        STAILQ_FOREACH(fs, files, next) {
            if (fs->fs_type != PS_FST_TYPE_SOCKET)
                continue;

            sockstat info{};
            char errbuf[256] = {};

            if (procstat_get_socket_info(
                    ps, fs, &info, errbuf
                ) != 0) {
                continue;
            }

            NetSocket s;

            s.local = freebsd_address(info.sa_local);
            s.remote = freebsd_address(info.sa_peer);

            if (info.proto == IPPROTO_TCP)
                s.protocol = "TCP";
            else if (info.proto == IPPROTO_UDP)
                s.protocol = "UDP";
            else
                continue;

            s.state = "UNKNOWN";
            s.pid = kp->ki_pid;

            char path[4096] = {};

            if (procstat_getpathname(
                    ps, kp, path, sizeof(path)
                ) == 0) {
                s.process_path = path;
            }

            char** argv = procstat_getargv(ps, kp, 0);

            if (argv) {
                for (size_t a = 0; argv[a]; ++a) {
                    if (a)
                        s.process_args += " ";

                    s.process_args += argv[a];
                }

                procstat_freeargv(ps);
            }

            out.push_back(std::move(s));
        }

        procstat_freefiles(ps, files);
    }

    procstat_freeprocs(ps, procs);
    procstat_close(ps);

    return true;
}

#endif

static void usage(const char* name) {
    std::cout
        << "Usage: " << name << " [OPTIONS]\n\n"
        << "Options:\n"
        << "  -p PORT    Show sockets matching local IP:PORT\n"
        << "  -i PID     Show sockets owned by PID\n"
        << "  -r PORT    Show process path and arguments for PORT\n"
        << "  -pp PORT   Print only the PID for PORT\n"
        << "  -o         Output pipe-delimited data\n"
        << "  -h         Show this help\n";
}

static bool matches_port(
    const NetSocket& s,
    const std::string& port
) {
    return s.local == port;
}

int main(int argc, char** argv) {
    std::string port_filter;
    std::string pid_filter;
    std::string process_filter;

    bool pid_only = false;
    bool output_pipe = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            return 0;
        }

        if (arg == "-pp") {
            if (++i >= argc) {
                usage(argv[0]);
                return 2;
            }

            port_filter = argv[i];
            pid_only = true;
        } else if (arg == "-p") {
            if (++i >= argc) {
                usage(argv[0]);
                return 2;
            }

            port_filter = argv[i];
        } else if (arg == "-i") {
            if (++i >= argc) {
                usage(argv[0]);
                return 2;
            }

            pid_filter = argv[i];
        } else if (arg == "-r") {
            if (++i >= argc) {
                usage(argv[0]);
                return 2;
            }

            process_filter = argv[i];
            port_filter = process_filter;
        } else if (arg == "-o") {
            output_pipe = true;
        } else {
            std::cerr << "netst: unknown option: " << arg << "\n";
            usage(argv[0]);
            return 2;
        }
    }

    if (!enumerate_sockets(sockets)) {
        std::cerr << "netst: socket enumeration was incomplete\n";
    }

    if (output_pipe) {
        std::cout
            << "protocol|local|remote|state|pid|process_path|process_args\n";
    }

    bool found = false;

    for (const auto& s : sockets) {
        if (!port_filter.empty() &&
            !matches_port(s, port_filter)) {
            continue;
        }

        if (!pid_filter.empty() &&
            std::to_string(s.pid) != pid_filter) {
            continue;
        }

        found = true;

        /*
         * Pipe-delimited output.
         */
        if (output_pipe) {
            std::cout
                << escape_field(s.protocol) << '|'
                << escape_field(s.local) << '|'
                << escape_field(s.remote) << '|'
                << escape_field(s.state) << '|'
                << (s.pid < 0
                    ? "unknown"
                    : std::to_string(s.pid)) << '|'
                << escape_field(
                    s.process_path.empty()
                        ? "unknown"
                        : s.process_path
                ) << '|'
                << escape_field(
                    s.process_args.empty()
                        ? "unknown"
                        : s.process_args
                ) << '\n';

            continue;
        }

        if (pid_only) {
            if (s.pid >= 0)
                std::cout << s.pid << '\n';

            continue;
        }

        if (!process_filter.empty()) {
            std::cout << "PORT: " << s.local << '\n';

            std::cout << " - PID: "
                      << (s.pid < 0
                          ? "unknown"
                          : std::to_string(s.pid))
                      << '\n';

            std::cout << " - Process path: "
                      << (s.process_path.empty()
                          ? "unavailable"
                          : s.process_path)
                      << '\n';

            std::cout << " - Process arguments: "
                      << (s.process_args.empty()
                          ? "unavailable"
                          : s.process_args)
                      << "\n\n";

            continue;
        }

        std::cout << "PORT: " << s.local << '\n';
        std::cout << " - Protocol: " << s.protocol << '\n';
        std::cout << " - State: " << s.state << '\n';
        std::cout << " - Remote: " << s.remote << '\n';

        std::cout << " - PID: "
                  << (s.pid < 0
                      ? "unknown"
                      : std::to_string(s.pid))
                  << '\n';

        std::cout << " - Process path: "
                  << (s.process_path.empty()
                      ? "unavailable"
                      : s.process_path)
                  << '\n';

        std::cout << " - Process arguments: "
                  << (s.process_args.empty()
                      ? "unavailable"
                      : s.process_args)
                  << "\n\n";
    }

    if (!found) {
        if (pid_only)
            return 1;

        std::cerr << "netst: no matching sockets found\n";
        return 1;
    }

    return 0;
}
