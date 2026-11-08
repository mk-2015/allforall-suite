#include <iostream>
#include <string>
#include <vector>

#if defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
    #include <tlhelp32.h>
    #include <winternl.h>
    #pragma comment(lib, "ntdll.lib")
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <sys/user.h>
    #include <unistd.h>
#else
    #include <fstream>
    #include <sstream>
    #include <unistd.h>
#endif

struct ProcessInfo {
    unsigned long pid = 0;
    unsigned long ppid = 0;
    std::string name;
    std::string exe_path;
    std::string command_line;
    unsigned long long memory_rss_kb = 0;
};

#if defined(_WIN32)
// --- WINDOWS IMPLEMENTATION ---
typedef NTSTATUS(WINAPI* pfnNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
);

ProcessInfo get_process_info(unsigned long pid) {
    ProcessInfo info;
    info.pid = pid;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return info;

    // 1. Executable Path
    char path_buf[MAX_PATH];
    DWORD path_size = MAX_PATH;
    if (QueryFullProcessImageNameA(hProcess, 0, path_buf, &path_size)) {
        info.exe_path = path_buf;
        size_t last_slash = info.exe_path.find_last_of("\\/");
        info.name = (last_slash != std::string::npos) ? info.exe_path.substr(last_slash + 1) : info.exe_path;
    }

    // 2. Memory Usage
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        info.memory_rss_kb = pmc.WorkingSetSize / 1024;
    }

    // 3. Command Line via NtQueryInformationProcess
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
        auto NtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(hNtdll, "NtQueryInformationProcess");
        if (NtQueryInformationProcess) {
            PROCESS_BASIC_INFORMATION pbi;
            if (NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL) >= 0 && pbi.PebBaseAddress) {
                // Read PEB and command line structures across process boundary...
                // (Simplified for brevity: Windows command line extraction requires reading remote PEB memory)
            }
        }
    }

    // 4. Parent PID via Toolhelp32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32{sizeof(PROCESSENTRY32)};
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == pid) {
                    info.ppid = pe32.th32ParentProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }

    CloseHandle(hProcess);
    return info;
}

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
// --- BSD FAMILY IMPLEMENTATION ---
ProcessInfo get_process_info(unsigned long pid) {
    ProcessInfo info;
    info.pid = pid;

    // 1. Basic Process Struct via kinfo_proc
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, static_cast<int>(pid)};
    struct kinfo_proc kp;
    size_t len = sizeof(kp);

    if (sysctl(mib, 4, &kp, &len, NULL, 0) == -1 || len == 0) {
        return info;
    }

#if defined(__FreeBSD__) || defined(__DragonFly__)
    info.name = kp.ki_comm;
    info.ppid = kp.ki_ppid;
    info.memory_rss_kb = kp.ki_rssize * (getpagesize() / 1024);
#elif defined(__OpenBSD__)
    info.name = kp.p_comm;
    info.ppid = kp.p_ppid;
    info.memory_rss_kb = kp.p_vm_rssize * (getpagesize() / 1024);
#elif defined(__NetBSD__)
    info.name = kp.p_comm;
    info.ppid = kp.p_pid;
#endif

    // 2. Executable Path via KERN_PROC_PATHNAME
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    int path_mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, static_cast<int>(pid)};
    char path_buf[1024];
    size_t path_len = sizeof(path_buf);
    if (sysctl(path_mib, 4, path_buf, &path_len, NULL, 0) == 0) {
        info.exe_path = path_buf;
    }
#elif defined(__OpenBSD__)
    // OpenBSD handles paths/args via KERN_PROC_ARGS
#endif

    // 3. Command Line Arguments via KERN_PROC_ARGS
    int args_mib[3];
#if defined(__FreeBSD__) || defined(__DragonFly__)
    args_mib[0] = CTL_KERN;
    args_mib[1] = KERN_PROC_ARGS;
    args_mib[2] = static_cast<int>(pid);
#elif defined(__NetBSD__)
    args_mib[0] = CTL_KERN;
    args_mib[1] = KERN_PROC_ARGS;
    args_mib[2] = static_cast<int>(pid);
    args_mib[3] = KERN_PROC_ARGV; // Specific sub-mib if needed
#endif

    // Query buffer size for arguments
    size_t arg_len = 0;
    // Note: KERN_PROC_ARGS format returns null-separated argument blocks string array
    if (sysctl(args_mib, 3, NULL, &arg_len, NULL, 0) == 0 && arg_len > 0) {
        std::vector<char> arg_buf(arg_len);
        if (sysctl(args_mib, 3, arg_buf.data(), &arg_len, NULL, 0) == 0) {
            // Parse null-separated arguments back into a single command line string
            for (size_t i = 0; i < arg_len; ++i) {
                char c = arg_buf[i];
                if (c == '\0') {
                    info.command_line += " ";
                } else {
                    info.command_line += c;
                }
            }
        }
    }

    return info;
}

#else
// --- LINUX IMPLEMENTATION ---
ProcessInfo get_process_info(unsigned long pid) {
    ProcessInfo info;
    info.pid = pid;
    std::string base_path = "/proc/" + std::to_string(pid);

    if (access(base_path.c_str(), F_OK) != 0) return info;

    // 1. Executable Path
    char symlink_buf[1024];
    ssize_t len = readlink((base_path + "/exe").c_str(), symlink_buf, sizeof(symlink_buf) - 1);
    if (len != -1) {
        symlink_buf[len] = '\0';
        info.exe_path = symlink_buf;
    }

    // 2. Name, PPID, Memory from status
    std::ifstream status_file(base_path + "/status");
    std::string line;
    while (std::getline(status_file, line)) {
        if (line.rfind("Name:", 0) == 0) {
            info.name = line.substr(5);
            info.name.erase(0, info.name.find_first_not_of(" \t"));
        } else if (line.rfind("PPid:", 0) == 0) {
            info.ppid = std::stoul(line.substr(5));
        } else if (line.rfind("VmRSS:", 0) == 0) {
            std::stringstream ss(line.substr(6));
            ss >> info.memory_rss_kb;
        }
    }

    // 3. Command Line Arguments
    std::ifstream cmd_file(base_path + "/cmdline", std::ios::binary);
    if (cmd_file.is_open()) {
        std::string arg;
        while (std::getline(cmd_file, arg, '\0')) {
            info.command_line += arg + " ";
        }
    }

    return info;
}
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << "psinfo" << " [PID]\n";
        return 1;
    }

    unsigned long target_pid = std::stoul(argv[1]);
    ProcessInfo info = get_process_info(target_pid);

    if (info.name.empty() && info.exe_path.empty()) {
        std::cerr << "Could not fetch details for PID " << target_pid << ".\n";
        return 1;
    }

    std::cout << "=== Process Inspection Report ===\n"
              << "PID:          " << info.pid << "\n"
              << "PPID:         " << info.ppid << "\n"
              << "Name:         " << info.name << "\n"
              << "Executable:   " << (info.exe_path.empty() ? "[Permission Denied/Unavailable]" : info.exe_path) << "\n"
              << "Command Line: " << (info.command_line.empty() ? "[Unavailable]" : info.command_line) << "\n"
              << "Memory RSS:   " << info.memory_rss_kb << " KB\n";

    return 0;
}