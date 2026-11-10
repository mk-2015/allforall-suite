#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <limits>

struct Options {
    std::string input;
    bool binary = false;
    bool json = false;
};

static void usage() {
    std::cout
        << "ipcacl - IP Subnet Calculator\n\n"
        << "Usage:\n"
        << "  ipcacl [OPTIONS] <IP>[/CIDR]\n"
        << "  ipcacl [OPTIONS] <IP> <NETMASK>\n\n"
        << "Options:\n"
        << "  -b, --binary   Show binary representations of IP and Netmask\n"
        << "  -j, --json     Output in JSON format\n"
        << "  -h, --help     Display this help message and exit\n\n"
        << "Examples:\n"
        << "  ipcacl 192.168.1.10/24\n"
        << "  ipcacl 10.0.0.5 255.255.255.0\n"
        << "  ipcacl -b 172.16.10.50/20\n";
}

static bool parse_ip(const std::string& str, uint32_t& out_ip) {
    std::stringstream ss(str);
    std::string segment;
    uint32_t ip = 0;
    int octets = 0;

    while (std::getline(ss, segment, '.')) {
        if (octets >= 4 || segment.empty())
            return false;
        
        for (char c : segment) {
            if (c < '0' || c > '9')
                return false;
        }

        try {
            int val = std::stoi(segment);
            if (val < 0 || val > 255)
                return false;
            ip = (ip << 8) | static_cast<uint32_t>(val);
            octets++;
        } catch (...) {
            return false;
        }
    }

    if (octets != 4)
        return false;

    out_ip = ip;
    return true;
}

static bool parse_netmask_to_cidr(const std::string& mask_str, int& out_cidr) {
    uint32_t mask = 0;
    if (!parse_ip(mask_str, mask))
        return false;

    // Count bits
    int cidr = 0;
    bool zero_found = false;
    for (int i = 31; i >= 0; --i) {
        if ((mask >> i) & 1) {
            if (zero_found)
                return false; // Invalid netmask (non-contiguous bits)
            cidr++;
        } else {
            zero_found = true;
        }
    }
    out_cidr = cidr;
    return true;
}

static std::string ip_to_string(uint32_t ip) {
    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << "."
       << ((ip >> 16) & 0xFF) << "."
       << ((ip >> 8) & 0xFF) << "."
       << (ip & 0xFF);
    return ss.str();
}

static std::string ip_to_binary(uint32_t ip) {
    std::stringstream ss;
    for (int i = 3; i >= 0; --i) {
        uint8_t octet = static_cast<uint8_t>((ip >> (i * 8)) & 0xFF);
        for (int b = 7; b >= 0; --b) {
            ss << ((octet >> b) & 1);
        }
        if (i > 0) ss << ".";
    }
    return ss.str();
}

int main(int argc, char* argv[]) {
    Options options;
    std::vector<std::string> args;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            usage();
            return 0;
        } else if (arg == "-b" || arg == "--binary") {
            options.binary = true;
        } else if (arg == "-j" || arg == "--json") {
            options.json = true;
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option '" << arg << "'\n";
            usage();
            return 1;
        } else {
            args.push_back(arg);
        }
    }

    if (args.empty() || args.size() > 2) {
        std::cerr << "Error: Invalid arguments.\n";
        usage();
        return 1;
    }

    uint32_t ip = 0;
    int cidr = 24;

    if (args.size() == 2) {
        // IP and Netmask (or CIDR)
        if (!parse_ip(args[0], ip)) {
            std::cerr << "Error: Invalid IP address '" << args[0] << "'\n";
            return 1;
        }
        std::string mask_arg = args[1];
        if (mask_arg[0] == '/') {
            mask_arg = mask_arg.substr(1);
        }
        try {
            int c = std::stoi(mask_arg);
            if (c < 0 || c > 32) {
                std::cerr << "Error: CIDR must be between 0 and 32.\n";
                return 1;
            }
            cidr = c;
        } catch (...) {
            if (!parse_netmask_to_cidr(mask_arg, cidr)) {
                std::cerr << "Error: Invalid netmask or CIDR '" << args[1] << "'\n";
                return 1;
            }
        }
    } else {
        // Single argument: IP/CIDR or IP
        std::string spec = args[0];
        size_t slash = spec.find('/');
        if (slash != std::string::npos) {
            std::string ip_str = spec.substr(0, slash);
            std::string cidr_str = spec.substr(slash + 1);
            if (!parse_ip(ip_str, ip)) {
                std::cerr << "Error: Invalid IP address '" << ip_str << "'\n";
                return 1;
            }
            try {
                int c = std::stoi(cidr_str);
                if (c < 0 || c > 32) {
                    std::cerr << "Error: CIDR must be between 0 and 32.\n";
                    return 1;
                }
                cidr = c;
            } catch (...) {
                std::cerr << "Error: Invalid CIDR specification '" << cidr_str << "'\n";
                return 1;
            }
        } else {
            if (!parse_ip(spec, ip)) {
                std::cerr << "Error: Invalid IP address '" << spec << "'\n";
                return 1;
            }
            cidr = 32; // Default to /32 if no CIDR specified
        }
    }

    uint32_t netmask = (cidr == 0) ? 0 : (0xFFFFFFFFUL << (32 - cidr));
    uint32_t wildcard = ~netmask;
    uint32_t network = ip & netmask;
    uint32_t broadcast = network | wildcard;

    uint32_t host_min = 0;
    uint32_t host_max = 0;
    uint64_t total_hosts = (cidr == 32) ? 1ULL : (1ULL << (32 - cidr));
    uint64_t usable_hosts = 0;

    if (cidr == 32) {
        host_min = ip;
        host_max = ip;
        usable_hosts = 1;
    } else if (cidr == 31) {
        host_min = network;
        host_max = broadcast;
        usable_hosts = 2;
    } else {
        host_min = network + 1;
        host_max = broadcast - 1;
        usable_hosts = total_hosts >= 2 ? total_hosts - 2 : 0;
    }

    if (options.json) {
        std::cout << "{\n"
                  << "  \"address\": \"" << ip_to_string(ip) << "\",\n"
                  << "  \"netmask\": \"" << ip_to_string(netmask) << "\",\n"
                  << "  \"wildcard\": \"" << ip_to_string(wildcard) << "\",\n"
                  << "  \"cidr\": " << cidr << ",\n"
                  << "  \"network\": \"" << ip_to_string(network) << "\",\n"
                  << "  \"broadcast\": \"" << ip_to_string(broadcast) << "\",\n"
                  << "  \"hostMin\": \"" << ip_to_string(host_min) << "\",\n"
                  << "  \"hostMax\": \"" << ip_to_string(host_max) << "\",\n"
                  << "  \"totalHosts\": " << total_hosts << ",\n"
                  << "  \"usableHosts\": " << usable_hosts << "\n"
                  << "}\n";
    } else {
        std::cout << "--------------------------------------------\n"
                  << " Address:     " << std::setw(15) << ip_to_string(ip) << "   "
                  << (options.binary ? ip_to_binary(ip) : "") << "\n"
                  << " Netmask:     " << std::setw(15) << ip_to_string(netmask) << " = /" << cidr
                  << (options.binary ? "   " + ip_to_binary(netmask) : "") << "\n"
                  << " Wildcard:    " << std::setw(15) << ip_to_string(wildcard) << "\n"
                  << " Network:     " << std::setw(15) << ip_to_string(network) << "\n"
                  << " Broadcast:   " << std::setw(15) << ip_to_string(broadcast) << "\n"
                  << " HostMin:     " << std::setw(15) << ip_to_string(host_min) << "\n"
                  << " HostMax:     " << std::setw(15) << ip_to_string(host_max) << "\n"
                  << " Total Hosts: " << std::setw(15) << total_hosts << "\n"
                  << " Usable Hosts:" << std::setw(15) << usable_hosts << "\n"
                  << "--------------------------------------------\n";
    }

    return 0;
}
