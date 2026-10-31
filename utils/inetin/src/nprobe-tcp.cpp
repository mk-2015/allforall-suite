
#include <boost/asio.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

using boost::asio::ip::tcp;
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
    bool connected = false;
    bool timed_out = false;
    double latency_ms = 0.0;
    std::string error;
};

static void usage()
{
    std::cout
        << "nprobe-tcp - TCP connectivity probe\n\n"
        << "Usage:\n"
        << "  nprobe-tcp [OPTIONS] HOST PORT\n\n"
        << "Options:\n"
        << "  -t COUNT       Number of connection attempts (default: 1)\n"
        << "  -w TIME        Connection timeout in seconds (default: 3)\n"
        << "  -i INTERVAL    Delay between attempts in milliseconds\n"
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
                std::cerr << "nprobe-tcp: " << arg
                          << " requires an argument\n";
                return false;
            }

            std::string value = argv[++i];

            if (arg == "-t") {
                if (!parse_uint(value, options.attempts)) {
                    std::cerr << "nprobe-tcp: invalid attempt count\n";
                    return false;
                }
            } else if (arg == "-w") {
                if (!parse_timeout(value, options.timeout)) {
                    std::cerr << "nprobe-tcp: invalid timeout\n";
                    return false;
                }
            } else {
                if (!parse_uint(value, options.interval, true)) {
                    std::cerr << "nprobe-tcp: invalid interval\n";
                    return false;
                }
            }

            continue;
        }

        if (!arg.empty() && arg[0] == '-') {
            std::cerr << "nprobe-tcp: unknown option: "
                      << arg << "\n";
            return false;
        }

        positional.push_back(arg);
    }

    if (positional.size() != 2) {
        std::cerr << "Usage: nprobe-tcp [OPTIONS] HOST PORT\n";
        return false;
    }

    options.host = positional[0];
    options.port = positional[1];

    unsigned int port;

    if (!parse_uint(options.port, port) || port > 65535) {
        std::cerr << "nprobe-tcp: invalid port\n";
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
    const tcp::resolver::results_type& endpoints,
    unsigned int attempt,
    double timeout_seconds)
{
    Result result;
    result.attempt = attempt;

    boost::asio::io_context io;
    tcp::socket socket(io);
    boost::asio::steady_timer timer(io);

    bool completed = false;
    Clock::time_point start = Clock::now();

    timer.expires_after(
        std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(timeout_seconds)));

    timer.async_wait([&](const boost::system::error_code& ec) {
        if (!ec && !completed) {
            completed = true;
            result.timed_out = true;

            boost::system::error_code ignored;
            socket.close(ignored);
        }
    });

    boost::asio::async_connect(
        socket,
        endpoints,
        [&](const boost::system::error_code& ec,
            const tcp::endpoint&) {

            if (completed)
                return;

            completed = true;
            timer.cancel();

            if (ec) {
                result.error = ec.message();
                return;
            }

            result.connected = true;

            boost::system::error_code ignored;
            socket.close(ignored);
        });

    io.run();

    // Measure every attempt, including failures and timeouts.
    result.latency_ms =
        std::chrono::duration<double, std::milli>(
            Clock::now() - start).count();

    if (result.timed_out)
        result.error = "Connection timed out";

    return result;
}


static void print_machine(const std::vector<Result>& results)
{
    std::cout << "attempt|status|latency_ms|error\n";

    for (const Result& result : results) {
        const char* status =
            result.connected ? "connected" :
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
        << "Protocol: TCP\n\n";

    std::vector<double> latencies;

    unsigned int successful = 0;
    unsigned int failed = 0;


    for (const Result& result : results) {
        std::cout << "Attempt " << result.attempt << ": ";

        if (result.connected) {
            ++successful;
            latencies.push_back(result.latency_ms);

            std::cout << "Connected";
        } else {
            ++failed;

            if (result.timed_out)
                std::cout << "Timeout";
            else
                std::cout << "Failed";
        }

        std::cout
            << " (" << std::fixed << std::setprecision(2)
            << result.latency_ms << " ms)";

        if (!result.error.empty())
            std::cout << " [" << result.error << "]";

        std::cout << "\n";
    }


    std::cout << "\nSummary:\n"
              << "  Attempts: " << results.size() << "\n"
              << "  Successful: " << successful << "\n"
              << "  Failed: " << failed << "\n";

    if (!latencies.empty()) {
        double minimum =
            *std::min_element(latencies.begin(), latencies.end());

        double maximum =
            *std::max_element(latencies.begin(), latencies.end());

        double average =
            std::accumulate(
                latencies.begin(), latencies.end(), 0.0)
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
        tcp::resolver resolver(io);

        boost::system::error_code ec;

        auto endpoints = resolver.resolve(
            options.host,
            options.port,
            ec);

        if (ec || endpoints.empty()) {
            std::cerr << "nprobe-tcp: DNS resolution failed: "
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
            if (!result.connected)
                return 1;
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "nprobe-tcp: " << e.what() << "\n";
        return 2;
    }
}
