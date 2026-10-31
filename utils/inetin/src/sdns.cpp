#include <ares.h>

#ifdef _WIN32
#  include <winsock2.h>
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#endif

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr int DNS_CLASS_IN = 1;
constexpr int TYPE_A = 1, TYPE_NS = 2, TYPE_CNAME = 5, TYPE_SOA = 6;
constexpr int TYPE_PTR = 12, TYPE_MX = 15, TYPE_TXT = 16, TYPE_AAAA = 28;

struct Options {
    std::string site, output_file, server;
    std::vector<std::string> servers;
    std::vector<int> types;
    bool reverse = false, explicit_type = false;
};

struct QueryResult {
    int type = 0;
    ares_status_t status = ARES_ECANCELLED;
    std::size_t timeouts = 0;
    bool completed = false;
    ares_dns_record_t* dns = nullptr;
};

const char* type_name(int type) {
    switch (type) {
        case TYPE_A: return "A"; case TYPE_AAAA: return "AAAA";
        case TYPE_CNAME: return "CNAME"; case TYPE_MX: return "MX";
        case TYPE_NS: return "NS"; case TYPE_TXT: return "TXT";
        case TYPE_PTR: return "PTR"; case TYPE_SOA: return "SOA";
        default: return "UNKNOWN";
    }
}

void help(const char* program) {
    std::cout << "Usage: " << program << " [OPTIONS] SITE\n\n"
              << "Default records: A, AAAA, CNAME\n"
              << "Supported: A, AAAA, CNAME, MX, NS, TXT, PTR, SOA\n\n"
              << "  -h              Show help\n"
              << "  -e RECORD       Query a record type (repeatable)\n"
              << "  -f FILE         Write output to FILE\n"
              << "  -s SERVER       Use one DNS server\n"
              << "  -ss SERVERS     Use servers: [\"1.1.1.1\", \"8.8.8.8\"]\n"
              << "  -x              Reverse lookup; SITE must be an IP address\n";
}

std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

bool record_type(const std::string& s, int& type) {
    const std::string n = upper(s);
    if (n == "A") type = TYPE_A;
    else if (n == "AAAA") type = TYPE_AAAA;
    else if (n == "CNAME") type = TYPE_CNAME;
    else if (n == "MX") type = TYPE_MX;
    else if (n == "NS") type = TYPE_NS;
    else if (n == "TXT") type = TYPE_TXT;
    else if (n == "PTR") type = TYPE_PTR;
    else if (n == "SOA") type = TYPE_SOA;
    else return false;
    return true;
}

std::string trim(std::string s) {
    const auto first = std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
    const auto last = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return first < last ? std::string(first, last) : std::string();
}

bool server_list(const std::string& input, std::vector<std::string>& out) {
    std::string token;
    char quote = 0;
    for (char c : input) {
        if (quote) {
            if (c == quote) quote = 0;
            else token.push_back(c);
        } else if (c == '\'' || c == '"') quote = c;
        else if (c == ',') {
            std::string item = trim(token); token.clear();
            if (!item.empty()) out.push_back(item);
        } else if (c != '[' && c != ']') token.push_back(c);
    }
    if (quote) return false;
    std::string item = trim(token);
    if (!item.empty()) out.push_back(item);
    return !out.empty();
}

bool arguments(int argc, char** argv, Options& o) {
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "-h" || a == "--help") { help(argv[0]); return false; }
        if (a == "-x") { o.reverse = true; continue; }
        if (a == "-e" || a == "-f" || a == "-s" || a == "-ss") {
            if (++i >= argc) { std::cerr << "sdns: missing argument for " << a << '\n'; return false; }
            const std::string v = argv[i];
            if (a == "-e") {
                int t = 0;
                if (!record_type(v, t)) { std::cerr << "sdns: unsupported record type: " << v << '\n'; return false; }
                o.types.push_back(t); o.explicit_type = true;
            } else if (a == "-f") o.output_file = v;
            else if (a == "-s") o.server = v;
            else if (!server_list(v, o.servers)) { std::cerr << "sdns: invalid -ss server list\n"; return false; }
            continue;
        }
        if (!a.empty() && a[0] == '-') { std::cerr << "sdns: unknown option: " << a << '\n'; return false; }
        if (!o.site.empty()) { std::cerr << "sdns: only one SITE is allowed\n"; return false; }
        o.site = a;
    }
    if (o.site.empty()) { std::cerr << "sdns: missing SITE\n"; return false; }
    if (!o.server.empty() && !o.servers.empty()) { std::cerr << "sdns: -s and -ss are mutually exclusive\n"; return false; }
    if (o.reverse) {
        if (o.explicit_type && (o.types.size() != 1 || o.types[0] != TYPE_PTR)) {
            std::cerr << "sdns: -x performs PTR; do not combine it with other record types\n"; return false;
        }
        o.types = {TYPE_PTR};
    } else if (o.types.empty()) o.types = {TYPE_A, TYPE_AAAA, TYPE_CNAME};
    return true;
}

bool reverse_name(const std::string& ip, std::string& name) {
    unsigned char bytes[16] = {};
    if (ares_inet_pton(AF_INET, ip.c_str(), bytes) == 1) {
        std::ostringstream s;
        s << unsigned(bytes[3]) << '.' << unsigned(bytes[2]) << '.'
          << unsigned(bytes[1]) << '.' << unsigned(bytes[0]) << ".in-addr.arpa";
        name = s.str(); return true;
    }
    if (ares_inet_pton(AF_INET6, ip.c_str(), bytes) == 1) {
        static constexpr char hex[] = "0123456789abcdef";
        for (int i = 15; i >= 0; --i) {
            name.push_back(hex[bytes[i] & 0x0f]); name.push_back('.');
            name.push_back(hex[(bytes[i] >> 4) & 0x0f]); name.push_back('.');
        }
        name += "ip6.arpa"; return true;
    }
    return false;
}

void query_callback(void* arg, ares_status_t status, std::size_t timeouts,
                    const ares_dns_record_t* dns) {
    auto* result = static_cast<QueryResult*>(arg);
    result->status = status;
    result->timeouts = timeouts;
    result->completed = true;
    if (dns) {
        result->dns = ares_dns_record_duplicate(dns);
        if (!result->dns) result->status = ARES_ENOMEM;
    }
}

std::string quoted(const unsigned char* data, std::size_t len) {
    std::ostringstream out; out << '"';
    for (std::size_t i = 0; i < len; ++i) {
        const unsigned char c = data[i];
        if (c == '"' || c == '\\') out << '\\' << char(c);
        else if (c >= 0x20 && c <= 0x7e) out << char(c);
        else out << "\\x" << std::hex << std::setw(2) << std::setfill('0') << unsigned(c) << std::dec;
    }
    return out.str() + '"';
}

const char* rr_string(const ares_dns_rr_t* rr, ares_dns_rr_key_t key) {
    const char* value = ares_dns_rr_get_str(rr, key);
    return value ? value : "<missing>";
}

void print_rr(std::ostream& out, const ares_dns_rr_t* rr) {
    const int type = static_cast<int>(ares_dns_rr_get_type(rr));
    const char* owner = ares_dns_rr_get_name(rr);
    const char* type_text = ares_dns_rec_type_tostr(static_cast<ares_dns_rec_type_t>(type));
    out << (owner ? owner : "?") << '\t' << ares_dns_rr_get_ttl(rr)
        << "\tIN\t" << (type_text ? type_text : type_name(type)) << '\t';
    switch (type) {
        case TYPE_A: {
            const in_addr* addr = ares_dns_rr_get_addr(rr, ARES_RR_A_ADDR);
            char text[64] = {};
            if (addr && ares_inet_ntop(AF_INET, addr, text, sizeof(text))) out << text;
            else out << "<invalid A>";
            break;
        }
        case TYPE_AAAA: {
            const ares_in6_addr* addr = ares_dns_rr_get_addr6(rr, ARES_RR_AAAA_ADDR);
            char text[128] = {};
            if (addr && ares_inet_ntop(AF_INET6, addr, text, sizeof(text))) out << text;
            else out << "<invalid AAAA>";
            break;
        }
        case TYPE_CNAME: out << rr_string(rr, ARES_RR_CNAME_CNAME); break;
        case TYPE_NS: out << rr_string(rr, ARES_RR_NS_NSDNAME); break;
        case TYPE_PTR: out << rr_string(rr, ARES_RR_PTR_DNAME); break;
        case TYPE_MX:
            out << ares_dns_rr_get_u16(rr, ARES_RR_MX_PREFERENCE) << ' '
                << rr_string(rr, ARES_RR_MX_EXCHANGE); break;
        case TYPE_TXT: {
            const std::size_t n = ares_dns_rr_get_abin_cnt(rr, ARES_RR_TXT_DATA);
            for (std::size_t i = 0; i < n; ++i) {
                std::size_t len = 0;
                const unsigned char* p = ares_dns_rr_get_abin(rr, ARES_RR_TXT_DATA, i, &len);
                if (i) out << ' ';
                if (p) out << quoted(p, len);
            }
            break;
        }
        case TYPE_SOA:
            out << rr_string(rr, ARES_RR_SOA_MNAME) << ' '
                << rr_string(rr, ARES_RR_SOA_RNAME) << ' '
                << ares_dns_rr_get_u32(rr, ARES_RR_SOA_SERIAL) << ' '
                << ares_dns_rr_get_u32(rr, ARES_RR_SOA_REFRESH) << ' '
                << ares_dns_rr_get_u32(rr, ARES_RR_SOA_RETRY) << ' '
                << ares_dns_rr_get_u32(rr, ARES_RR_SOA_EXPIRE) << ' '
                << ares_dns_rr_get_u32(rr, ARES_RR_SOA_MINIMUM); break;
        default: out << "<unformatted record>"; break;
    }
    out << '\n';
}

bool print_response(std::ostream& out, const QueryResult& q) {
    if (!q.dns) {
        out << "  " << ares_strerror(q.status) << '\n';
        return false;
    }
    const char* rcode = ares_dns_rcode_tostr(ares_dns_record_get_rcode(q.dns));
    out << "  Response: " << (rcode ? rcode : "unknown")
        << ", timeouts: " << q.timeouts << '\n';
    const ares_dns_section_t sections[] = {
        ARES_SECTION_ANSWER, ARES_SECTION_AUTHORITY, ARES_SECTION_ADDITIONAL
    };
    const char* labels[] = {"Answer", "Authority", "Additional"};
    bool found = false;
    for (int s = 0; s < 3; ++s) {
        const std::size_t count = ares_dns_record_rr_cnt(q.dns, sections[s]);
        if (!count) continue;
        out << "  [" << labels[s] << "]\n";
        for (std::size_t i = 0; i < count; ++i) {
            const ares_dns_rr_t* rr = ares_dns_record_rr_get_const(q.dns, sections[s], i);
            if (rr) { print_rr(out, rr); found = true; }
        }
    }
    if (!found) out << "  No resource records returned.\n";
    return true;
}

bool run_server(const std::string& server, const Options& o,
                const std::string& name, std::ostream& out) {
    ares_channel_t* channel = nullptr;
    ares_options opts = {};
    opts.evsys = ARES_EVSYS_DEFAULT;
    opts.timeout = 2500;
    opts.tries = 2;
    const int mask = ARES_OPT_EVENT_THREAD | ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES;
    int st = ares_init_options(&channel, &opts, mask);
    if (st != ARES_SUCCESS) { out << "sdns: c-ares init: " << ares_strerror(st) << '\n'; return false; }
    if (!server.empty()) {
        st = ares_set_servers_csv(channel, server.c_str());
        if (st != ARES_SUCCESS) {
            out << "sdns: invalid DNS server " << server << ": " << ares_strerror(st) << '\n';
            ares_destroy(channel); return false;
        }
    }
    std::vector<QueryResult> results(o.types.size());
    for (std::size_t i = 0; i < o.types.size(); ++i) {
        results[i].type = o.types[i];
        const ares_status_t queued = ares_query_dnsrec(
            channel, name.c_str(), static_cast<ares_dns_class_t>(DNS_CLASS_IN),
            static_cast<ares_dns_rec_type_t>(o.types[i]), query_callback, &results[i], nullptr);
        if (queued != ARES_SUCCESS && !results[i].completed)
            results[i].status = queued;
    }
    st = ares_queue_wait_empty(channel, 20000);
    ares_destroy(channel);
    if (st != ARES_SUCCESS) {
        for (const auto& q : results)
            if (q.dns) ares_dns_record_destroy(q.dns);
        out << "sdns: query wait: " << ares_strerror(st) << '\n';
        return false;
    }
    bool any = false;
    for (const auto& q : results) {
        out << "Query: " << name << "  Type: " << type_name(q.type) << '\n';
        if (q.status != ARES_SUCCESS && q.status != ARES_ENODATA)
            out << "  Query status: " << ares_strerror(q.status) << '\n';
        if (print_response(out, q)) any = true;
        if (q.dns) ares_dns_record_destroy(q.dns);
    }
    return any;
}
} // namespace

int main(int argc, char** argv) {
    Options o;
    if (!arguments(argc, argv, o)) {
        if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) return 0;
        return 2;
    }
    std::string query = o.site;
    if (o.reverse && !reverse_name(o.site, query)) {
        std::cerr << "sdns: -x requires a valid IPv4 or IPv6 address\n"; return 2;
    }
    int st = ares_library_init(ARES_LIB_INIT_ALL);
    if (st != ARES_SUCCESS) { std::cerr << "sdns: " << ares_strerror(st) << '\n'; return 1; }
    if (!ares_threadsafety()) {
        std::cerr << "sdns: this c-ares build lacks thread support required for event-thread mode\n";
        ares_library_cleanup();
        return 1;
    }

    std::vector<std::string> servers;
    if (!o.server.empty()) servers.push_back(o.server);
    else if (!o.servers.empty()) servers = o.servers;
    else servers.emplace_back(); // c-ares uses the system resolver configuration.

    std::ofstream text_file;
    std::ostream* output = &std::cout;
    if (!o.output_file.empty()) {
        text_file.open(o.output_file, std::ios::out | std::ios::trunc);
        if (!text_file) { std::cerr << "sdns: cannot open " << o.output_file << '\n'; ares_library_cleanup(); return 1; }
        output = &text_file;
    }

    bool success = true;
    for (const auto& server : servers) {
        if (!server.empty() || servers.size() > 1)
            *output << "DNS server: " << (server.empty() ? "system configuration" : server) << '\n';
        const bool ok = run_server(server, o, query, *output);
        success = ok && success;
    }
    ares_library_cleanup();
    return success ? 0 : 1;
}
