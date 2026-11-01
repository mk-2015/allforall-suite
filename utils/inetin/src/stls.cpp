
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <memory>
#include <iomanip>
#include <chrono>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MSYS__)
    #define STLS_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define CLOSE_SOCKET closesocket
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET_VALUE (-1)
    using socket_t = int;
#endif

struct Site {
    std::string host;
    std::string port = "443";
    bool ip_literal = false;
};

struct Options {
    bool verbose = false;
    bool certificate = false;
    bool json = false;
    bool verify = true;

    std::string port;
    std::string sni;
    int timeout = 10;

    std::string site;
};

static void help()
{
    std::cout <<
        "stls [OPTIONS] [SITE]\n"
        "\n"
        "View TLS information about a specific site.\n"
        "\n"
        "Options:\n"
        "  -h              Display help\n"
        "  -v              Verbose output\n"
        "  -c              Show certificate details\n"
        "  -j              JSON output\n"
        "  -p PORT         Specify port (default: 443)\n"
        "  -s HOST         Specify SNI hostname\n"
        "  -t SECONDS      Connection timeout (default: 10)\n"
        "  --no-verify     Disable certificate verification\n";
}

static bool valid_port(const std::string& value)
{
    if (value.empty())
        return false;

    unsigned long port = 0;

    for (unsigned char c : value) {
        if (!std::isdigit(c))
            return false;

        port = port * 10 + (c - '0');

        if (port > 65535)
            return false;
    }

    return port > 0;
}

static bool is_ip(const std::string& host)
{
    unsigned char buffer[16];

    return inet_pton(AF_INET, host.c_str(), buffer) == 1 ||
           inet_pton(AF_INET6, host.c_str(), buffer) == 1;
}

static bool parse_site(const std::string& input, Site& site)
{
    if (input.empty())
        return false;

    std::string authority = input;

    const auto scheme = authority.find("://");

    if (scheme != std::string::npos) {
        std::string protocol = authority.substr(0, scheme);

        std::transform(
            protocol.begin(), protocol.end(),
            protocol.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        );

        if (protocol != "https")
            return false;

        authority.erase(0, scheme + 3);
    }

    const auto end = authority.find_first_of("/?#");

    if (end != std::string::npos)
        authority.erase(end);

    if (authority.empty() ||
        authority.find('@') != std::string::npos)
        return false;

    if (authority.front() == '[') {
        const auto close = authority.find(']');

        if (close == std::string::npos)
            return false;

        site.host = authority.substr(1, close - 1);

        unsigned char ipv6_buffer[16];

        if (inet_pton(AF_INET6, site.host.c_str(),
                    ipv6_buffer) != 1)
            return false;

        const std::string remainder =
            authority.substr(close + 1);

        if (!remainder.empty()) {
            if (remainder.front() != ':')
                return false;

            site.port = remainder.substr(1);
        }

        site.ip_literal = true;
    }
    else {
        const auto first = authority.find(':');
        const auto last = authority.rfind(':');

        if (first != std::string::npos && first == last) {
            site.host = authority.substr(0, first);
            site.port = authority.substr(first + 1);
        }
        else {
            site.host = authority;
        }

        site.ip_literal = is_ip(site.host);
    }

    if (site.host.empty() || !valid_port(site.port))
        return false;

    return true;
}

static bool parse_args(int argc, char** argv, Options& opt)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            help();
            std::exit(0);
        }
        else if (arg == "-v") {
            opt.verbose = true;
        }
        else if (arg == "-c") {
            opt.certificate = true;
        }
        else if (arg == "-j") {
            opt.json = true;
        }
        else if (arg == "--no-verify") {
            opt.verify = false;
        }
        else if (arg == "-p" || arg == "-s" || arg == "-t") {
            if (++i >= argc)
                return false;

            std::string value = argv[i];

            if (arg == "-p") {
                if (!valid_port(value))
                    return false;

                opt.port = value;
            }
            else if (arg == "-s") {
                opt.sni = value;
            }
            else {
                try {
                    size_t used = 0;
                    int timeout = std::stoi(value, &used);

                    if (used != value.size() ||
                        timeout < 1 || timeout > 3600)
                        return false;

                    opt.timeout = timeout;
                }
                catch (...) {
                    return false;
                }
            }
        }
        else if (!arg.empty() && arg[0] == '-') {
            return false;
        }
        else if (opt.site.empty()) {
            opt.site = arg;
        }
        else {
            return false;
        }
    }

    return true;
}

#ifdef STLS_WINDOWS
class SocketInit {
public:
    bool ok;

    SocketInit() {
        WSADATA data;
        ok = WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }

    ~SocketInit() {
        if (ok)
            WSACleanup();
    }
};
#endif

static bool set_nonblocking(socket_t sock, bool enabled)
{
#ifdef STLS_WINDOWS
    u_long mode = enabled ? 1 : 0;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(sock, F_GETFL, 0);

    if (flags < 0)
        return false;

    if (enabled)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    return fcntl(sock, F_SETFL, flags) == 0;
#endif
}

/*
 * Capture the exact certificate verification error while OpenSSL
 * is performing certificate-chain and hostname verification.
 */
static int capture_verify_error(
    int preverify_ok,
    X509_STORE_CTX* store)
{
    if (!preverify_ok) {
        SSL* ssl = static_cast<SSL*>(
            X509_STORE_CTX_get_ex_data(
                store,
                SSL_get_ex_data_X509_STORE_CTX_idx()
            )
        );

        if (ssl) {
            auto* saved = static_cast<long*>(
                SSL_get_app_data(ssl)
            );

            if (saved)
                *saved = X509_STORE_CTX_get_error(store);
        }
    }

    return preverify_ok;
}

static bool connect_socket(
    socket_t sock,
    const sockaddr* address,
    socklen_t length,
    int timeout)
{
    if (!set_nonblocking(sock, true))
        return false;

    int result = connect(sock, address, length);

    if (result != 0) {
#ifdef STLS_WINDOWS
        int error = WSAGetLastError();

        if (error != WSAEWOULDBLOCK &&
            error != WSAEINPROGRESS)
            return false;
#else
        if (errno != EINPROGRESS)
            return false;
#endif
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);

    timeval tv{};
    tv.tv_sec = timeout;

#ifdef STLS_WINDOWS
    result = select(0, nullptr, &writefds, nullptr, &tv);
#else
    result = select(sock + 1, nullptr, &writefds, nullptr, &tv);
#endif

    if (result <= 0)
        return false;

    int error = 0;
    socklen_t error_length = sizeof(error);

    if (getsockopt(sock, SOL_SOCKET, SO_ERROR,
                   reinterpret_cast<char*>(&error),
                   &error_length) != 0 || error != 0)
        return false;

    return set_nonblocking(sock, false);
}

static socket_t connect_host(
    const std::string& host,
    const std::string& port,
    int timeout)
{
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* results = nullptr;

    int status = getaddrinfo(
        host.c_str(), port.c_str(), &hints, &results
    );

    if (status != 0)
        return INVALID_SOCKET_VALUE;

    socket_t connected = INVALID_SOCKET_VALUE;

    for (addrinfo* p = results; p; p = p->ai_next) {
        socket_t sock = socket(
            p->ai_family, p->ai_socktype, p->ai_protocol
        );

        if (sock == INVALID_SOCKET_VALUE)
            continue;

        if (connect_socket(
                sock, p->ai_addr,
                static_cast<socklen_t>(p->ai_addrlen),
                timeout)) {
            connected = sock;
            break;
        }

        CLOSE_SOCKET(sock);
    }

    freeaddrinfo(results);
    return connected;
}

enum class HandshakeStatus {
    Success,
    Timeout,
    Failed
};

static HandshakeStatus tls_handshake(
    SSL* ssl,
    socket_t sock,
    int timeout,
    int& ssl_error)
{
    using Clock = std::chrono::steady_clock;

    const auto deadline =
        Clock::now() + std::chrono::seconds(timeout);

    ssl_error = 0;

    if (!set_nonblocking(sock, true))
        return HandshakeStatus::Failed;

    while (true) {
        ERR_clear_error();

        int result = SSL_connect(ssl);

        if (result == 1) {
            set_nonblocking(sock, false);
            return HandshakeStatus::Success;
        }

        // Must be called immediately after SSL_connect().
        ssl_error = SSL_get_error(ssl, result);

        bool want_read = ssl_error == SSL_ERROR_WANT_READ;
        bool want_write = ssl_error == SSL_ERROR_WANT_WRITE;

        if (!want_read && !want_write)
            return HandshakeStatus::Failed;

        auto now = Clock::now();

        if (now >= deadline)
            return HandshakeStatus::Timeout;

        auto remaining =
            std::chrono::duration_cast<
                std::chrono::microseconds
            >(deadline - now);

        timeval tv{};

        tv.tv_sec = static_cast<long>(remaining.count() / 1000000);
        tv.tv_usec = static_cast<long>(remaining.count() % 1000000);

        fd_set readfds;
        fd_set writefds;

        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        if (want_read)
            FD_SET(sock, &readfds);

        if (want_write)
            FD_SET(sock, &writefds);

#ifdef STLS_WINDOWS
        int selected = select(
            0,
            want_read ? &readfds : nullptr,
            want_write ? &writefds : nullptr,
            nullptr,
            &tv
        );
#else
        int selected = select(
            sock + 1,
            want_read ? &readfds : nullptr,
            want_write ? &writefds : nullptr,
            nullptr,
            &tv
        );
#endif

        if (selected > 0)
            continue;

        if (selected == 0)
            return HandshakeStatus::Timeout;

#ifdef STLS_WINDOWS
        if (WSAGetLastError() == WSAEINTR)
            continue;
#else
        if (errno == EINTR)
            continue;
#endif

        return HandshakeStatus::Failed;
    }
}

static std::string bio_string(BIO* bio)
{
    BUF_MEM* memory = nullptr;
    BIO_get_mem_ptr(bio, &memory);

    if (!memory || !memory->data)
        return {};

    return std::string(memory->data, memory->length);
}

static std::string name_string(X509_NAME* name)
{
    BIO* bio = BIO_new(BIO_s_mem());

    if (!bio)
        return {};

    X509_NAME_print_ex(bio, name, 0, XN_FLAG_RFC2253);

    std::string result = bio_string(bio);
    BIO_free(bio);

    return result;
}

static std::string date_string(const ASN1_TIME* time)
{
    struct tm tm_value{};

    if (ASN1_TIME_to_tm(time, &tm_value) != 1)
        return "Unknown";

    static const char* months[] = {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    if (tm_value.tm_mon < 0 || tm_value.tm_mon > 11)
        return "Unknown";

    char buffer[64];

    std::snprintf(
        buffer, sizeof(buffer),
        "%s %02d %02d:%02d:%02d %04d GMT",
        months[tm_value.tm_mon],
        tm_value.tm_mday,
        tm_value.tm_hour,
        tm_value.tm_min,
        tm_value.tm_sec,
        tm_value.tm_year + 1900
    );

    return buffer;
}

static std::string json_escape(const std::string& value)
{
    std::string result;

    for (unsigned char c : value) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c < 32) {
                    std::ostringstream out;
                    out << "\\u"
                        << std::hex << std::setw(4)
                        << std::setfill('0')
                        << static_cast<int>(c);
                    result += out.str();
                }
                else {
                    result += static_cast<char>(c);
                }
        }
    }

    return result;
}

static void print_certificate(
    X509* cert,
    bool json)
{
    std::string subject =
        name_string(X509_get_subject_name(cert));

    std::string issuer =
        name_string(X509_get_issuer_name(cert));

    std::string from =
        date_string(X509_get0_notBefore(cert));

    std::string until =
        date_string(X509_get0_notAfter(cert));

    if (json) {
        std::cout
            << "\"certificate\":{"
            << "\"subject\":\"" << json_escape(subject) << "\","
            << "\"issuer\":\"" << json_escape(issuer) << "\","
            << "\"valid_from\":\"" << json_escape(from) << "\","
            << "\"valid_until\":\"" << json_escape(until) << "\""
            << "}";
        return;
    }

    std::cout
        << "Certificate:\n"
        << "  Subject: " << subject << "\n"
        << "  Issuer: " << issuer << "\n"
        << "  Valid From: " << from << "\n"
        << "  Valid Until: " << until << "\n";
}

int main(int argc, char** argv)
{
    Options opt;

    if (!parse_args(argc, argv, opt)) {
        std::cerr << "Invalid arguments. Use -h for help.\n";
        return 2;
    }

    if (opt.site.empty()) {
        std::cerr << "Missing SITE.\n";
        return 2;
    }

    Site site;

    if (!parse_site(opt.site, site)) {
        std::cerr << "Invalid SITE.\n";
        return 2;
    }

    if (!opt.port.empty())
        site.port = opt.port;

    const std::string identity =
        opt.sni.empty() ? site.host : opt.sni;

#ifdef STLS_WINDOWS
    SocketInit socket_init;

    if (!socket_init.ok) {
        std::cerr << "Winsock initialization failed.\n";
        return 2;
    }
#endif

    long verification_error = X509_V_OK;

    SSL_CTX* raw_ctx = SSL_CTX_new(TLS_client_method());

    if (!raw_ctx) {
        std::cerr << "Could not initialize OpenSSL.\n";
        return 2;
    }

    std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>
        ctx(raw_ctx, SSL_CTX_free);

    if (opt.verify) {
        /*
         * IMPORTANT:
         * Keep capture_verify_error installed.
         * Do not replace it with nullptr.
         */
        SSL_CTX_set_verify(
            ctx.get(),
            SSL_VERIFY_PEER,
            capture_verify_error
        );

        if (SSL_CTX_set_default_verify_paths(ctx.get()) != 1) {
            std::cerr << "Could not load default CA certificates.\n";
            return 2;
        }
    }
    else {
        SSL_CTX_set_verify(
            ctx.get(),
            SSL_VERIFY_NONE,
            nullptr
        );
    }

    socket_t sock = connect_host(
        site.host, site.port, opt.timeout
    );

    if (sock == INVALID_SOCKET_VALUE) {
        std::cerr << "TCP connection failed.\n";
        return 1;
    }

    SSL* raw_ssl = SSL_new(ctx.get());

    if (!raw_ssl) {
        CLOSE_SOCKET(sock);
        std::cerr << "Could not create TLS session.\n";
        return 2;
    }

    std::unique_ptr<SSL, decltype(&SSL_free)>
        ssl(raw_ssl, SSL_free);

    if (SSL_set_fd(ssl.get(), static_cast<int>(sock)) != 1) {
        CLOSE_SOCKET(sock);
        std::cerr << "Could not attach socket to TLS session.\n";
        return 2;
    }

    /*
     * The callback retrieves this pointer during certificate
     * verification. It remains valid until the handshake ends.
     */
    SSL_set_app_data(ssl.get(), &verification_error);

    if (opt.verify) {
        X509_VERIFY_PARAM* param =
            SSL_get0_param(ssl.get());

        unsigned char ip_buffer[16];

        if (inet_pton(AF_INET, identity.c_str(), ip_buffer) == 1 ||
            inet_pton(AF_INET6, identity.c_str(), ip_buffer) == 1) {
            if (X509_VERIFY_PARAM_set1_ip_asc(
                    param, identity.c_str()) != 1) {
                CLOSE_SOCKET(sock);
                std::cerr << "Could not configure IP verification.\n";
                return 2;
            }
        }
        else {
            if (SSL_set1_host(ssl.get(), identity.c_str()) != 1) {
                CLOSE_SOCKET(sock);
                std::cerr << "Could not configure hostname verification.\n";
                return 2;
            }
        }
    }

    unsigned char alpn[] = {
        2, 'h', '2',
        8, 'h', 't', 't', 'p', '/', '1', '.', '1'
    };

    if (SSL_set_alpn_protos(
            ssl.get(), alpn, sizeof(alpn)) != 0) {
        CLOSE_SOCKET(sock);
        std::cerr << "Could not configure ALPN.\n";
        return 2;
    }

    unsigned char ip_buffer[16];

    bool identity_is_ip =
        inet_pton(AF_INET, identity.c_str(), ip_buffer) == 1 ||
        inet_pton(AF_INET6, identity.c_str(), ip_buffer) == 1;

    if (!identity_is_ip) {
        if (SSL_set_tlsext_host_name(
                ssl.get(), identity.c_str()) != 1) {
            CLOSE_SOCKET(sock);
            std::cerr << "Could not configure SNI.\n";
            return 2;
        }
    }

    ERR_clear_error();

    int handshake_error = 0;

    HandshakeStatus handshake_status =
        tls_handshake(
            ssl.get(),
            sock,
            opt.timeout,
            handshake_error
        );

    if (handshake_status != HandshakeStatus::Success) {
        std::cerr << "TLS handshake failed: ";

        if (handshake_status == HandshakeStatus::Timeout) {
            std::cerr
                << "Timed out after "
                << opt.timeout
                << " seconds.";
        }
        else {
            /*
             * Prefer the exact error captured by the verification
             * callback. Fall back to SSL_get_verify_result().
             */
            long verify_result = verification_error;

            if (verify_result == X509_V_OK)
                verify_result = SSL_get_verify_result(ssl.get());

            if (verify_result != X509_V_OK) {
                std::cerr
                    << "Certificate verification failed: "
                    << X509_verify_cert_error_string(verify_result);
            }
            else {
                unsigned long code = ERR_get_error();

                if (code)
                    std::cerr << ERR_error_string(code, nullptr);
                else
                    std::cerr << "SSL error " << handshake_error;
            }
        }

        std::cerr << '\n';

        CLOSE_SOCKET(sock);
        return 1;
    }

    const char* version = SSL_get_version(ssl.get());

    const SSL_CIPHER* cipher =
        SSL_get_current_cipher(ssl.get());

    const char* cipher_name =
        cipher ? SSL_CIPHER_get_name(cipher) : "Unknown";

    const unsigned char* alpn_selected = nullptr;
    unsigned int alpn_length = 0;

    SSL_get0_alpn_selected(
        ssl.get(), &alpn_selected, &alpn_length
    );

    std::string alpn_name = "None";

    if (alpn_length)
        alpn_name.assign(
            reinterpret_cast<const char*>(alpn_selected),
            alpn_length
        );

    long verify_result = SSL_get_verify_result(ssl.get());

    X509* cert = SSL_get1_peer_certificate(ssl.get());

    if (opt.json) {
        std::cout
            << "{"
            << "\"site\":\"" << json_escape(site.host) << "\","
            << "\"port\":" << site.port << ","
            << "\"tls_version\":\"" << json_escape(version) << "\","
            << "\"cipher\":\"" << json_escape(cipher_name) << "\","
            << "\"alpn\":\"" << json_escape(alpn_name) << "\","
            << "\"verification\":\""
            << (opt.verify
                ? (verify_result == X509_V_OK ? "OK" : "FAILED")
                : "DISABLED")
            << "\"";

        if (cert && opt.certificate) {
            std::cout << ",";
            print_certificate(cert, true);
        }

        std::cout << "}\n";
    }
    else {
        std::cout
            << "Site: "
            << (site.host.find(':') != std::string::npos
                    ? "[" + site.host + "]"
                    : site.host)
            << ":" << site.port << "\n\n"
            << "TLS:\n"
            << "  Version: " << version << "\n"
            << "  Cipher: " << cipher_name << "\n"
            << "  ALPN: " << alpn_name << "\n\n"
            << "Verification:\n"
            << "  Status: ";

        if (!opt.verify)
            std::cout << "DISABLED\n";
        else if (verify_result == X509_V_OK)
            std::cout << "OK\n";
        else
            std::cout << "FAILED: "
                      << X509_verify_cert_error_string(verify_result)
                      << "\n";

        if (cert && opt.certificate) {
            std::cout << "\n";
            print_certificate(cert, false);
        }

        if (opt.verbose) {
            std::cout
                << "\nProtocol: TLS client\n"
                << "OpenSSL: " << OpenSSL_version(OPENSSL_VERSION)
                << "\n";
        }
    }

    if (cert)
        X509_free(cert);

    SSL_shutdown(ssl.get());
    CLOSE_SOCKET(sock);

    return opt.verify && verify_result != X509_V_OK ? 1 : 0;
}
