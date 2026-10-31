
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct QueryParam {
    std::string name;
    std::string value;
    bool has_equal = false;
};

struct URL {
    std::string protocol;
    std::string host;
    std::string userinfo;
    int port = -1;
    std::string path;
    std::vector<QueryParam> query;
    std::string fragment;
    bool has_query = false;
};

static bool is_unreserved(unsigned char c)
{
    return std::isalnum(c) ||
           c == '-' || c == '.' ||
           c == '_' || c == '~';
}

static bool is_path_reserved(unsigned char c)
{
    const std::string reserved = "!$&'()*+,;=:@/";
    return reserved.find(static_cast<char>(c)) != std::string::npos;
}

static std::string url_encode(const std::string& input, bool path_mode = false)
{
    static const char hex[] = "0123456789ABCDEF";

    std::string output;

    for (unsigned char c : input) {
        if (is_unreserved(c) || (path_mode && is_path_reserved(c))) {
            output += static_cast<char>(c);
        } else {
            output += '%';
            output += hex[c >> 4];
            output += hex[c & 0x0F];
        }
    }

    return output;
}

static int hex_value(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    return -1;
}

static bool url_decode(const std::string& input, std::string& output)
{
    output.clear();

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] != '%') {
            output += input[i];
            continue;
        }

        if (i + 2 >= input.size())
            return false;

        int hi = hex_value(input[i + 1]);
        int lo = hex_value(input[i + 2]);

        if (hi < 0 || lo < 0)
            return false;

        output += static_cast<char>((hi << 4) | lo);
        i += 2;
    }

    return true;
}

static bool parse_port(const std::string& value, int& port)
{
    if (value.empty())
        return false;

    for (char c : value) {
        if (!std::isdigit(static_cast<unsigned char>(c)))
            return false;
    }

    try {
        unsigned long n = std::stoul(value);

        if (n > 65535)
            return false;

        port = static_cast<int>(n);
        return true;
    } catch (...) {
        return false;
    }
}

static bool parse_query(
    const std::string& input,
    std::vector<QueryParam>& output)
{
    output.clear();

    if (input.empty())
        return true;

    size_t start = 0;

    while (start <= input.size()) {
        size_t end = input.find('&', start);

        if (end == std::string::npos)
            end = input.size();

        std::string item = input.substr(start, end - start);

        if (!item.empty()) {
            size_t equal = item.find('=');

            QueryParam param;

            if (equal == std::string::npos) {
                param.has_equal = false;

                if (!url_decode(item, param.name))
                    return false;
            } else {
                param.has_equal = true;

                if (!url_decode(item.substr(0, equal), param.name))
                    return false;

                if (!url_decode(item.substr(equal + 1), param.value))
                    return false;
            }

            output.push_back(param);
        }

        if (end == input.size())
            break;

        start = end + 1;
    }

    return true;
}

static bool valid_scheme(const std::string& scheme)
{
    if (scheme.empty() ||
        !std::isalpha(static_cast<unsigned char>(scheme[0])))
        return false;

    for (char c : scheme) {
        if (!std::isalnum(static_cast<unsigned char>(c)) &&
            c != '+' && c != '-' && c != '.')
            return false;
    }

    return true;
}

static bool parse_authority(const std::string& authority, URL& url)
{
    if (authority.empty())
        return false;

    std::string hostport = authority;

    size_t at = authority.rfind('@');

    if (at != std::string::npos) {
        url.userinfo = authority.substr(0, at);
        hostport = authority.substr(at + 1);
    }

    if (hostport.empty())
        return false;

    if (hostport[0] == '[') {
        size_t close = hostport.find(']');

        if (close == std::string::npos)
            return false;

        url.host = hostport.substr(1, close - 1);

        std::string remainder = hostport.substr(close + 1);

        if (!remainder.empty()) {
            if (remainder[0] != ':')
                return false;

            if (!parse_port(remainder.substr(1), url.port))
                return false;
        }
    } else {
        size_t colon = hostport.rfind(':');

        if (colon != std::string::npos) {
            // An unbracketed authority with multiple colons
            // is ambiguous and is rejected.
            if (hostport.find(':') != colon)
                return false;

            url.host = hostport.substr(0, colon);

            if (!parse_port(hostport.substr(colon + 1), url.port))
                return false;
        } else {
            url.host = hostport;
        }
    }

    return !url.host.empty();
}

static bool url_parse(const std::string& input, URL& url)
{
    url = URL{};

    std::string remaining = input;

    // Fragment
    size_t fragment_pos = remaining.find('#');

    if (fragment_pos != std::string::npos) {
        url.fragment = remaining.substr(fragment_pos + 1);
        remaining.resize(fragment_pos);
    }

    // Query
    size_t query_pos = remaining.find('?');

    if (query_pos != std::string::npos) {
        url.has_query = true;

        std::string query_string = remaining.substr(query_pos + 1);
        remaining.resize(query_pos);

        if (!parse_query(query_string, url.query))
            return false;
    }

    // Scheme and authority
    size_t scheme_pos = remaining.find("://");

    if (scheme_pos != std::string::npos) {
        url.protocol = remaining.substr(0, scheme_pos);

        if (!valid_scheme(url.protocol))
            return false;

        std::string authority_and_path =
            remaining.substr(scheme_pos + 3);

        size_t slash = authority_and_path.find('/');

        std::string authority;

        if (slash == std::string::npos) {
            authority = authority_and_path;
            url.path = "/";
        } else {
            authority = authority_and_path.substr(0, slash);
            url.path = authority_and_path.substr(slash);
        }

        if (!parse_authority(authority, url))
            return false;
    } else {
        url.path = remaining;
    }

    return true;
}

static std::string url_build(const URL& url)
{
    std::string output;

    if (!url.protocol.empty())
        output += url.protocol + "://";

    if (!url.userinfo.empty())
        output += url.userinfo + "@";

    if (url.host.find(':') != std::string::npos)
        output += "[" + url.host + "]";
    else
        output += url.host;

    if (url.port >= 0)
        output += ":" + std::to_string(url.port);

    output += url.path;

    if (url.has_query || !url.query.empty()) {
        output += "?";

        for (size_t i = 0; i < url.query.size(); ++i) {
            if (i)
                output += "&";

            const QueryParam& p = url.query[i];

            output += url_encode(p.name);

            if (p.has_equal)
                output += "=" + url_encode(p.value);
        }
    }

    if (!url.fragment.empty())
        output += "#" + url.fragment;

    return output;
}

static void print_usage()
{
    std::cout <<
        "furl - URL utility\n"
        "\n"
        "Usage:\n"
        "  furl -e TEXT\n"
        "  furl -d TEXT\n"
        "  furl -qp URL\n"
        "  furl -qp NAME URL\n"
        "  furl -h URL\n"
        "  furl -p URL\n"
        "  furl -P URL\n"
        "  furl -q URL\n"
        "  furl -C PROTOCOL HOST PATH [NAME=VALUE ...] [-Cp PORT]\n"
        "\n"
        "Options:\n"
        "  -e       Percent-encode text\n"
        "  -d       Percent-decode text\n"
        "  -qp      Extract all query parameters\n"
        "  -qp NAME Extract values of a query parameter\n"
        "  -h       Extract host\n"
        "  -p       Extract port\n"
        "  -P       Extract protocol\n"
        "  -q       Extract path\n"
        "  -C       Construct a URL\n"
        "  -Cp PORT Set port when constructing a URL\n"
        "  --help   Show this help\n";
}

enum class Operation {
    NONE,
    ENCODE,
    DECODE,
    QUERY,
    HOST,
    PORT,
    PROTOCOL,
    PATH,
    CONSTRUCT
};

int main(int argc, char* argv[])
{
    Operation operation = Operation::NONE;

    std::string text;
    std::string query_name;

    URL url;

    bool have_text = false;
    bool have_query_name = false;
    bool construct = false;

    std::string construct_protocol;
    std::string construct_host;
    std::string construct_path;

    std::vector<QueryParam> construct_query;

    int construct_port = -1;

    auto fail = [](const std::string& message) {
        std::cerr << "furl: " << message << "\n";
        return 2;
    };

    auto set_operation = [&](Operation op) -> bool {
        if (operation != Operation::NONE)
            return false;

        operation = op;
        return true;
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help") {
            print_usage();
            return 0;
        }

        if (arg == "-Cp") {
            if (!construct || i + 1 >= argc)
                return fail("-Cp requires -C and a port");

            if (!parse_port(argv[++i], construct_port))
                return fail("invalid port");

            continue;
        }

        if (arg == "-C") {
            if (!set_operation(Operation::CONSTRUCT))
                return fail("only one operation can be used");

            construct = true;

            if (i + 3 >= argc)
                return fail("-C requires PROTOCOL HOST PATH");

            construct_protocol = argv[++i];
            construct_host = argv[++i];
            construct_path = argv[++i];

            if (!construct_protocol.empty() &&
                !valid_scheme(construct_protocol))
                return fail("invalid protocol");

            continue;
        }

        if (arg == "-e") {
            if (!set_operation(Operation::ENCODE))
                return fail("only one operation can be used");
            continue;
        }

        if (arg == "-d") {
            if (!set_operation(Operation::DECODE))
                return fail("only one operation can be used");
            continue;
        }

        if (arg == "-qp") {
            if (!set_operation(Operation::QUERY))
                return fail("only one operation can be used");
            continue;
        }

        if (arg == "-h") {
            if (!set_operation(Operation::HOST))
                return fail("only one operation can be used");
            continue;
        }

        if (arg == "-p") {
            if (!set_operation(Operation::PORT))
                return fail("only one operation can be used");
            continue;
        }

        if (arg == "-P") {
            if (!set_operation(Operation::PROTOCOL))
                return fail("only one operation can be used");
            continue;
        }

        if (arg == "-q") {
            if (!set_operation(Operation::PATH))
                return fail("only one operation can be used");
            continue;
        }

        if (construct) {
            size_t equal = arg.find('=');

            QueryParam param;

            if (equal == std::string::npos) {
                param.name = arg;
                param.has_equal = false;
            } else {
                param.name = arg.substr(0, equal);
                param.value = arg.substr(equal + 1);
                param.has_equal = true;
            }

            construct_query.push_back(param);
            continue;
        }

        if (operation == Operation::QUERY) {
            if (!have_text) {
                text = arg;
                have_text = true;
            } else if (!have_query_name) {
                query_name = text;
                have_query_name = true;
                text = arg;
            } else {
                return fail("too many arguments");
            }
            continue;
        }

        if (have_text)
            return fail("too many arguments");

        text = arg;
        have_text = true;
    }

    if (operation == Operation::NONE)
        return fail("no operation specified; use --help");

    if (operation == Operation::CONSTRUCT) {
        url.protocol = construct_protocol;
        url.host = construct_host;
        url.path = construct_path;
        url.port = construct_port;
        url.query = construct_query;
        url.has_query = !construct_query.empty();

        if (url.path.empty() && !url.host.empty())
            url.path = "/";

        if (url.host.find(':') != std::string::npos &&
            !(url.host.front() == '[' && url.host.back() == ']')) {
            // Keep the host internally unbracketed.
        } else if (url.host.size() >= 2 &&
                   url.host.front() == '[' &&
                   url.host.back() == ']') {
            url.host = url.host.substr(1, url.host.size() - 2);
        }

        if (url.host.empty())
            return fail("host cannot be empty");

        if (!url.protocol.empty() && !valid_scheme(url.protocol))
            return fail("invalid protocol");

        std::cout << url_build(url) << "\n";
        return 0;
    }

    if (operation == Operation::QUERY) {
        if (!have_text)
            return fail("usage: furl -qp [NAME] URL");

        std::string target = text;

        if (!url_parse(target, url))
            return fail("invalid URL or query encoding");

        bool found = false;

        for (const QueryParam& param : url.query) {
            if (have_query_name && param.name != query_name)
                continue;

            found = true;

            std::cout << param.name;

            if (param.has_equal)
                std::cout << "=" << param.value;

            std::cout << "\n";
        }

        return found || !have_query_name ? 0 : 1;
    }

    if (!have_text)
        return fail("missing TEXT or URL");

    if (operation == Operation::ENCODE) {
        std::cout << url_encode(text) << "\n";
        return 0;
    }

    if (operation == Operation::DECODE) {
        std::string decoded;

        if (!url_decode(text, decoded))
            return fail("invalid percent-encoding");

        std::cout << decoded << "\n";
        return 0;
    }

    if (!url_parse(text, url))
        return fail("invalid URL");

    switch (operation) {
        case Operation::HOST:
            if (url.host.empty())
                return 1;
            std::cout << url.host << "\n";
            break;

        case Operation::PORT:
            if (url.port < 0)
                return 1;
            std::cout << url.port << "\n";
            break;

        case Operation::PROTOCOL:
            if (url.protocol.empty())
                return 1;
            std::cout << url.protocol << "\n";
            break;

        case Operation::PATH:
            std::cout << url.path << "\n";
            break;

        default:
            return fail("unsupported operation");
    }

    return 0;
}
