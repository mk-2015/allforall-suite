
#include <boost/asio.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

enum class AddressMode {
    IPv4,
    IPv6,
    All
};

static void usage()
{
    std::cout <<
        "Usage: resolv [OPTIONS] [SITE]\n"
        "\n"
        "DNS Resolution command.\n"
        "\n"
        "OPTIONS:\n"
        "  -h       Show help\n"
        "  -4       Output IPv4 addresses (default)\n"
        "  -6       Output IPv6 addresses\n"
        "  -a       Output all IPv4 and IPv6 addresses\n"
        "  -r       Reverse DNS: IP address to hostname\n";
}

// Accept either a hostname or URL.
static std::string extract_host(std::string input)
{
    auto scheme = input.find("://");

    if (scheme != std::string::npos)
        input.erase(0, scheme + 3);

    auto end = input.find_first_of("/?#");

    if (end != std::string::npos)
        input.resize(end);

    // Remove user information.
    auto at = input.rfind('@');

    if (at != std::string::npos)
        input.erase(0, at + 1);

    // Handle bracketed IPv6 addresses.
    if (!input.empty() && input.front() == '[') {
        auto close = input.find(']');

        if (close != std::string::npos)
            return input.substr(1, close - 1);
    }

    // Remove port from ordinary hostname or IPv4.
    auto colon = input.rfind(':');

    if (colon != std::string::npos &&
        input.find(':') == colon)
        input.resize(colon);

    return input;
}

static void print_unique(
    const std::vector<std::string>& addresses)
{
    std::unordered_set<std::string> seen;

    for (const auto& address : addresses) {
        if (seen.insert(address).second)
            std::cout << address << '\n';
    }
}

static std::vector<std::string> resolve_forward(
    asio::io_context& ctx,
    const std::string& host,
    AddressMode mode)
{
    tcp::resolver resolver(ctx);
    std::vector<std::string> addresses;

    auto collect = [&](const auto& results) {
        for (const auto& entry : results) {
            addresses.push_back(
                entry.endpoint().address().to_string());
        }
    };

    auto resolve_family = [&](auto protocol) {
        try {
            collect(resolver.resolve(protocol, host, "0"));
            return true;
        }
        catch (const boost::system::system_error& e) {
            std::cerr << "resolv: "
                      << (protocol == tcp::v4() ? "IPv4" : "IPv6")
                      << ": " << e.code().message() << '\n';
            return false;
        }
    };

    bool success = false;

    if (mode == AddressMode::IPv4 ||
        mode == AddressMode::All)
    {
        success |= resolve_family(tcp::v4());
    }

    if (mode == AddressMode::IPv6 ||
        mode == AddressMode::All)
    {
        success |= resolve_family(tcp::v6());
    }

    if (!success)
        throw std::runtime_error("all address lookups failed");

    return addresses;
}

static std::vector<std::string> resolve_reverse(
    asio::io_context& ctx,
    const std::string& ip)
{
    tcp::resolver resolver(ctx);

    auto address = asio::ip::make_address(ip);
    tcp::endpoint endpoint(address, 0);

    auto results = resolver.resolve(endpoint);

    std::vector<std::string> hostnames;

    for (const auto& entry : results)
        hostnames.push_back(entry.host_name());

    return hostnames;
}

int main(int argc, char** argv)
{
    AddressMode mode = AddressMode::IPv4;

    bool reverse = false;
    bool mode_selected = false;

    std::string site;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            usage();
            return 0;
        }
        else if (arg == "-4" ||
                 arg == "-6" ||
                 arg == "-a")
        {
            if (mode_selected) {
                std::cerr
                    << "resolv: conflicting address options\n";
                return 2;
            }

            mode_selected = true;

            if (arg == "-4")
                mode = AddressMode::IPv4;
            else if (arg == "-6")
                mode = AddressMode::IPv6;
            else
                mode = AddressMode::All;
        }
        else if (arg == "-r") {
            reverse = true;
        }
        else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "resolv: unknown option: "
                      << arg << '\n';
            return 2;
        }
        else {
            if (!site.empty()) {
                std::cerr
                    << "resolv: too many SITE arguments\n";
                return 2;
            }

            site = arg;
        }
    }

    if (site.empty()) {
        std::cerr << "resolv: missing SITE\n";
        std::cerr << "Try 'resolv -h' for usage.\n";
        return 2;
    }

    site = extract_host(site);

    if (site.empty()) {
        std::cerr << "resolv: invalid SITE\n";
        return 2;
    }

    try {
        asio::io_context ctx;

        std::vector<std::string> results;

        if (reverse)
            results = resolve_reverse(ctx, site);
        else
            results = resolve_forward(ctx, site, mode);

        if (results.empty()) {
            std::cerr << "resolv: no results found\n";
            return 1;
        }

        print_unique(results);
        return 0;
    }
    catch (const boost::system::system_error& e) {
        std::cerr << "resolv: " << e.code().message () << '\n';
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "resolv: " << e.what() << '\n';
        return 1;
    }
}
