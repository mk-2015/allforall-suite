
#include <boost/asio.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>

using boost::asio::ip::udp;
using Clock = std::chrono::steady_clock;

struct Options {
    std::string host;
    std::string port;

    unsigned int attempts = 1;
    double timeout = 3.0;
    unsigned int interval = 0;

    bool machine = false;
};

struct Result {
    unsigned int attempt = 0;
    bool responded = false;
    bool timed_out = false;
    double latency_ms = 0.0;
    std::string error;
};

static void usage()
{
    std::cout
        << "nprobe-udp - UDP connectivity probe\n\n"
        << "Usage:\n"
        << "  nprobe-udp [OPTIONS] HOST PORT\n\n"
        << "Options:\n"
        << "  -t COUNT       Number of probes (default: 1)\n"
        << "  -w TIME        Response timeout in seconds (default: 3)\n"
        << "  -i INTERVAL    Delay between probes in milliseconds\n"
        << "  -o             Machine-readable output\n"
        << "  -h             Show this help\n";
}

static bool parse_uint(
    const std::string& value,
    unsigned int& output,
    bool allow_zero = false)
{
    if (value.empty())
        return false;

    for (char c : value) {
        if (c < '0' || c > '9')
            return false;
    }

    try {
        unsigned long n = std::stoul(value);

        if (n > std::numeric_limits<unsigned int>::max())
            return false;

        if (!allow_zero && n == 0)
            return false;

        output = static_cast<unsigned int>(n);
        return true;
    } catch (...) {
        return false;
    }
}

static bool parse_timeout(const std::string& value, double& output)
{
    if (value.empty())
        return false;

    char* end = nullptr;
    double n = std::strtod(value.c_str(), &end);

    if (end == value.c_str() || *end != '\0')
        return false;

    if (!std::isfinite(n) || n <= 0)
        return false;

    output = n;
    return true;
}

static bool parse_arguments(int argc, char* argv[], Options& options)
{
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            usage();
            std::exit(0);
        }

        if (arg == "-o") {
            options.machine = true;
            continue;
        }

        if (arg == "-t" || arg == "-w" || arg == "-i") {
            if (i + 1 >= argc) {
                std::cerr << "nprobe-udp: " << arg
                          << " requires an argument\n";
                return false;
            }

            std::string value = argv[++i];

            if (arg == "-t") {
                if (!parse_uint(value, options.attempts)) {
                    std::cerr << "nprobe-udp: invalid attempt count\n";
                    return false;
                }
            } else if (arg == "-w") {
                if (!parse_timeout(value, options.timeout)) {
                    std::cerr << "nprobe-udp: invalid timeout\n";
                    return false;
                }
            } else {
                if (!parse_uint(value, options.interval, true)) {
                    std::cerr << "nprobe-udp: invalid interval\n";
                    return false;
                }
            }

            continue;
        }

        if (!arg.empty() && arg[0] == '-') {
            std::cerr << "nprobe-udp: unknown option: "
                      << arg << "\n";
            return false;
        }

        positional.push_back(arg);
    }

    if (positional.size() != 2) {
        std::cerr << "Usage: nprobe-udp [OPTIONS] HOST PORT\n";
        return false;
    }

    options.host = positional[0];
    options.port = positional[1];

    unsigned int port;

    if (!parse_uint(options.port, port) || port > 65535) {
        std::cerr << "nprobe-udp: invalid port\n";
        return false;
    }

    return true;
}

static std::string escape_field(const std::string& input)
{
    std::string output;

    for (char c : input) {
        switch (c) {
            case '\\': output += "\\\\"; break;
            case '|':  output += "\\|";  break;
            case '\n': output += "\\n";  break;
            case '\r': output += "\\r";  break;
            case '\t': output += "\\t";  break;
            default:   output += c;      break;
        }
    }

    return output;
}

static Result probe(
    const udp::resolver::results_type& endpoints,
    unsigned int attempt,
    double timeout_seconds)
{
    Result result;
    result.attempt = attempt;

    boost::asio::io_context io;
    udp::socket socket(io);

    boost::system::error_code ec;
    bool prepared = false;

    // Try the resolved addresses until one can be used.
    for (auto it = endpoints.begin(); it != endpoints.end(); ++it) {
        socket.open(it->endpoint().protocol(), ec);

        if (ec)
            continue;

        socket.connect(it->endpoint(), ec);

        if (!ec) {
            prepared = true;
            break;
        }

        boost::system::error_code ignored;
        socket.close(ignored);
    }

    if (!prepared) {
        result.error = ec ? ec.message() : "No usable endpoint";
        return result;
    }

    boost::asio::steady_timer timer(io);
    std::array<char, 2048> response{};

    bool completed = false;
    const char probe_data[] = "";

    Clock::time_point start = Clock::now();

    timer.expires_after(
        std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(timeout_seconds)));

    timer.async_wait([&](const boost::system::error_code& timer_ec) {
        if (!timer_ec && !completed) {
            completed = true;
            result.timed_out = true;

            boost::system::error_code ignored;
            socket.close(ignored);
        }
    });

    socket.async_send(
        boost::asio::buffer(probe_data, 0),
        [&](const boost::system::error_code& send_ec,
            std::size_t) {

            if (completed)
                return;

            if (send_ec) {
                completed = true;
                timer.cancel();
                result.error = send_ec.message();
                return;
            }

            socket.async_receive(
                boost::asio::buffer(response),
                [&](const boost::system::error_code& receive_ec,
                    std::size_t) {

                    if (completed)
                        return;

                    completed = true;
                    timer.cancel();

                    if (receive_ec) {
                        result.error = receive_ec.message();
                        return;
                    }

                    result.responded = true;
                });
        });

    io.run();

    result.latency_ms =
        std::chrono::duration<double, std::milli>(
            Clock::now() - start).count();

    if (result.timed_out)
        result.error = "No response before timeout";

    return result;
}

static void print_machine(const std::vector<Result>& results)
{
    std::cout << "attempt|status|latency_ms|error\n";

    for (const Result& result : results) {
        const char* status =
            result.responded ? "responded" :
            result.timed_out ? "timeout" : "failed";

        std::cout
            << result.attempt << "|"
            << status << "|"
            << std::fixed << std::setprecision(2)
            << result.latency_ms << "|"
            << escape_field(result.error)
            << "\n";
    }
}

static void print_human(
    const Options& options,
    const std::vector<Result>& results)
{
    std::cout
        << "Host: " << options.host << "\n"
        << "Port: " << options.port << "\n"
        << "Protocol: UDP\n\n";

    std::vector<double> latencies;

    unsigned int responded = 0;
    unsigned int timed_out = 0;
    unsigned int failed = 0;

    for (const Result& result : results) {
        std::cout << "Attempt " << result.attempt << ": ";

        if (result.responded) {
            ++responded;
            latencies.push_back(result.latency_ms);
            std::cout << "Responded";
        } else if (result.timed_out) {
            ++timed_out;
            std::cout << "Timeout";
        } else {
            ++failed;
            std::cout << "Failed";
        }

        std::cout
            << " (" << std::fixed << std::setprecision(2)
            << result.latency_ms << " ms)";

        if (!result.error.empty())
            std::cout << " [" << result.error << "]";

        std::cout << "\n";
    }

    std::cout
        << "\nSummary:\n"
        << "  Attempts: " << results.size() << "\n"
        << "  Responded: " << responded << "\n"
        << "  Timed out: " << timed_out << "\n"
        << "  Failed: " << failed << "\n";

    if (!latencies.empty()) {
        double minimum =
            *std::min_element(latencies.begin(), latencies.end());

        double maximum =
            *std::max_element(latencies.begin(), latencies.end());

        double average =
            std::accumulate(latencies.begin(), latencies.end(), 0.0)
            / latencies.size();

        std::cout
            << "  Minimum: " << minimum << " ms\n"
            << "  Maximum: " << maximum << " ms\n"
            << "  Average: " << average << " ms\n";
    }
}

int main(int argc, char* argv[])
{
    Options options;

    if (!parse_arguments(argc, argv, options))
        return 2;

    try {
        boost::asio::io_context io;
        udp::resolver resolver(io);

        boost::system::error_code ec;

        auto endpoints = resolver.resolve(
            options.host,
            options.port,
            ec);

        if (ec || endpoints.empty()) {
            std::cerr << "nprobe-udp: DNS resolution failed: "
                      << (ec ? ec.message() : "no endpoints found")
                      << "\n";
            return 2;
        }

        std::vector<Result> results;
        results.reserve(options.attempts);

        for (unsigned int i = 1; i <= options.attempts; ++i) {
            results.push_back(
                probe(endpoints, i, options.timeout));

            if (i < options.attempts && options.interval > 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(options.interval));
            }
        }

        if (options.machine)
            print_machine(results);
        else
            print_human(options, results);

        for (const Result& result : results) {
            if (!result.responded)
                return 1;
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "nprobe-udp: " << e.what() << "\n";
        return 2;
    }
}
