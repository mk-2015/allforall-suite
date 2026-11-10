#include <boost/asio.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <memory>
#include <thread>
#include <iomanip>
#include <mutex>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

struct Options {
    std::string target; // CIDR or IP
    double timeout = 1.0;
    std::vector<int> ports = {80, 443, 22};
    std::string format = "text"; // text, json, toml, yaml
    bool help = false;
};

struct HostResult {
    std::string ip;
    bool alive = false;
    int port_matched = 0;
    double latency_ms = 0.0;
};

static void usage() {
    std::cout
        << "hostdescv - IPv4/IPv6 Host Discovery Tool\n\n"
        << "Usage:\n"
        << "  hostdescv [OPTIONS] <CIDR or IP>\n\n"
        << "Options:\n"
        << "  -p, `--ports` PORTS    Comma-separated ports to probe (default: 80,443,22)\n"
        << "  -w, `--timeout` SEC    Connection timeout in seconds (default: 1.0)\n"
        << "  -o, `--format` FMT     Output format: text, json, toml, yaml (default: text)\n"
        << "  -h, `--help`           Display this help message and exit\n\n"
        << "Examples:\n"
        << "  hostdescv 192.168.1.0/24\n"
        << "  hostdescv -o json 10.0.0.0/28\n"
        << "  hostdescv -p 80,443 -o yaml 192.168.1.50\n";
}

// Simple IPv4 CIDR expansion
static bool parse_ipv4_cidr(const std::string& cidr_str, std::vector<std::string>& out_ips) {
    size_t slash = cidr_str.find('/');
    std::string ip_part = (slash != std::string::npos) ? cidr_str.substr(0, slash) : cidr_str;
    int prefix = (slash != std::string::npos) ? std::stoi(cidr_str.substr(slash + 1)) : 32;

    if (prefix < 0 || prefix > 32)
        return false;

    // Parse IP
    std::stringstream ss(ip_part);
    std::string seg;
    uint32_t ip = 0;
    int octets = 0;
    while (std::getline(ss, seg, '.')) {
        if (octets >= 4 || seg.empty()) return false;
        int val = std::stoi(seg);
        if (val < 0 || val > 255) return false;
        ip = (ip << 8) | static_cast<uint32_t>(val);
        octets++;
    }
    if (octets != 4) return false;

    uint32_t netmask = (prefix == 0) ? 0 : (0xFFFFFFFFUL << (32 - prefix));
    uint32_t network = ip & netmask;
    uint64_t total = (prefix == 32) ? 1ULL : (1ULL << (32 - prefix));

    // Limit total expansion for safety in subnet scanning (e.g. max /16 = 65536, but let's cap or allow)
    if (total > 65536) {
        std::cerr << "Error: Subnet too large (max /16 supported for scanning).\n";
        return false;
    }

    for (uint64_t i = 0; i < total; ++i) {
        uint32_t target_ip = network + static_cast<uint32_t>(i);
        // For subnets >= /24, optionally skip network and broadcast, or include all
        std::stringstream ips;
        ips << ((target_ip >> 24) & 0xFF) << "."
            << ((target_ip >> 16) & 0xFF) << "."
            << ((target_ip >> 8) & 0xFF) << "."
            << (target_ip & 0xFF);
        out_ips.push_back(ips.str());
    }

    return true;
}

class ProbeTask : public std::enable_shared_from_this<ProbeTask> {
    asio::io_context& io;
    std::string ip;
    int port;
    double timeout_sec;
    std::function<void(bool, double)> callback;

    tcp::socket socket;
    asio::steady_timer timer;
    std::chrono::steady_clock::time_point start_time;
    bool finished = false;

public:
    ProbeTask(asio::io_context& context, const std::string& target_ip, int target_port, double timeout, std::function<void(bool, double)> cb)
        : io(context), ip(target_ip), port(target_port), timeout_sec(timeout), callback(cb), socket(context), timer(context) {}

    void start() {
        auto self = shared_from_this();
        start_time = std::chrono::steady_clock::now();

        timer.expires_after(std::chrono::milliseconds(static_cast<long>(timeout_sec * 1000)));
        timer.async_wait([self](const boost::system::error_code& ec) {
            if (!ec) self->finish(false);
        });

        boost::system::error_code ec;
        auto address = asio::ip::make_address(ip, ec);
        if (ec) {
            finish(false);
            return;
        }

        tcp::endpoint endpoint(address, static_cast<unsigned short>(port));
        socket.async_connect(endpoint, [self](const boost::system::error_code& ec) {
            if (self->finished) return;
            auto end_time = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end_time - self->start_time;
            self->finish(!ec, elapsed.count());
        });
    }

private:
    void finish(bool success, double latency = 0.0) {
        if (finished) return;
        finished = true;
        timer.cancel();
        boost::system::error_code ignored_ec;
        socket.close(ignored_ec);
        callback(success, latency);
    }
};

int main(int argc, char* argv[]) {
    Options options;
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            usage();
            return 0;
        } else if ((arg == "-p" || arg == "--ports") && i + 1 < argc) {
            std::string ports_str = argv[++i];
            options.ports.clear();
            std::stringstream ss(ports_str);
            std::string p;
            while (std::getline(ss, p, ',')) {
                try {
                    options.ports.push_back(std::stoi(p));
                } catch (...) {}
            }
        } else if ((arg == "-w" || arg == "--timeout") && i + 1 < argc) {
            try {
                options.timeout = std::stod(argv[++i]);
            } catch (...) {}
        } else if ((arg == "-o" || arg == "--format") && i + 1 < argc) {
            options.format = argv[++i];
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option '" << arg << "'\n";
            usage();
            return 1;
        } else {
            positional.push_back(arg);
        }
    }

    if (positional.empty()) {
        std::cerr << "Error: Target CIDR or IP address required.\n";
        usage();
        return 1;
    }

    options.target = positional[0];

    std::vector<std::string> target_ips;
    if (!parse_ipv4_cidr(options.target, target_ips)) {
        // Fallback to single IP if CIDR parsing failed
        target_ips.push_back(options.target);
    }

    asio::io_context io;
    std::vector<HostResult> results;
    std::mutex results_mutex;
    int active_probes = 0;
    std::mutex counter_mutex;

    for (const auto& ip : target_ips) {
        for (int port : options.ports) {
            {
                std::lock_guard<std::mutex> lock(counter_mutex);
                active_probes++;
            }

            auto task = std::make_shared<ProbeTask>(io, ip, port, options.timeout,
                [&results, &results_mutex, &active_probes, &counter_mutex, ip, port](bool success, double latency) {
                    if (success) {
                        std::lock_guard<std::mutex> lock(results_mutex);
                        // Check if host already discovered
                        bool found = false;
                        for (auto& r : results) {
                            if (r.ip == ip) {
                                found = true;
                                if (latency < r.latency_ms) r.latency_ms = latency;
                                break;
                            }
                        }
                        if (!found) {
                            results.push_back({ip, true, port, latency});
                        }
                    }
                    std::lock_guard<std::mutex> lock(counter_mutex);
                    active_probes--;
                });

            task->start();
        }
    }

    // Run io_context in background thread or run directly until all probes finish or timeout
    // Since timers/async connect are asynchronous, let's run io.run_for or poll with a timeout loop
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        io.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        {
            std::lock_guard<std::mutex> lock(counter_mutex);
            if (active_probes == 0) break;
        }

        auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count();
        if (elapsed > (options.timeout + 2.0)) {
            io.stop();
            break;
        }
    }

    // Output formatting
    if (options.format == "json") {
        std::cout << "[\n";
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "  {\n"
                      << "    \"ip\": \"" << results[i].ip << "\",\n"
                      << "    \"alive\": " << (results[i].alive ? "true" : "false") << ",\n"
                      << "    \"port\": " << results[i].port_matched << ",\n"
                      << "    \"latencyMs\": " << std::fixed << std::setprecision(2) << results[i].latency_ms << "\n"
                      << "  }" << (i + 1 < results.size() ? "," : "") << "\n";
        }
        std::cout << "]\n";
    } else if (options.format == "toml") {
        std::cout << "# Host Discovery Results\n\n";
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "[[host]]\n"
                      << "ip = \"" << results[i].ip << "\"\n"
                      << "alive = true\n"
                      << "port = " << results[i].port_matched << "\n"
                      << "latency_ms = " << std::fixed << std::setprecision(2) << results[i].latency_ms << "\n\n";
        }
    } else if (options.format == "yaml") {
        std::cout << "hosts:\n";
        for (const auto& r : results) {
            std::cout << "  - ip: \"" << r.ip << "\"\n"
                      << "    alive: true\n"
                      << "    port: " << r.port_matched << "\n"
                      << "    latency_ms: " << std::fixed << std::setprecision(2) << r.latency_ms << "\n";
        }
    } else {
        // Text format
        std::cout << "----------------------------------------\n"
                  << " Host Discovery Results (" << results.size() << " hosts alive)\n"
                  << "----------------------------------------\n";
        for (const auto& r : results) {
            std::cout << " [ACTIVE] " << std::setw(15) << r.ip
                      << " (Port: " << r.port_matched
                      << ", Latency: " << std::fixed << std::setprecision(2) << r.latency_ms << " ms)\n";
        }
        std::cout << "----------------------------------------\n";
    }

    return 0;
}
