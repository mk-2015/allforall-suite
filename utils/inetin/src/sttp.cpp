#include <curl/curl.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

struct Config {
    std::string site;
    std::string method = "GET";
    std::string body;
    bool body_specified = false;
    bool download_option_seen = false;
    std::string user_agent;
    std::vector<std::pair<std::string, std::string>> headers;

    std::string download_file;
    std::string upload_file;
    std::string response_headers_file;

    bool download_existing_only = false;
    bool download_remote_name = false;
    bool follow_redirects = false;
    bool show_response_headers = false;
    bool verbose = false;
    bool force_ipv4 = false;
    bool force_ipv6 = false;

    long timeout_ms = 0;
    long connect_timeout_ms = 0;
    long max_redirects = -1;
};

struct Transfer {
    Config* config = nullptr;
    FILE* output = nullptr;
    FILE* headers_output = nullptr;
    FILE* upload = nullptr;
    std::string remote_filename;
    std::string content_type;
    std::string server;
    curl_off_t downloaded = 0;
    curl_off_t uploaded = 0;
    bool output_opened = false;
    bool output_error = false;
};

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool parse_long(const char* text, long& value) {
    if (!text || !*text) return false;
    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(text, &end, 10);
    if (errno || !end || *end != '\0' || parsed < 0) return false;
    value = parsed;
    return true;
}

void usage(std::ostream& out) {
    out <<
"sttp [OPTIONS] [SITE]\n"
"\n"
"Make a request to SITE using the request modified by OPTIONS.\n"
"Simple HTTP.\n"
"\n"
"Methods: GET, HEAD, POST, PUT, DELETE, OPTIONS, TRACE, PATCH\n"
"\n"
"Options:\n"
"  -m METHOD       HTTP method (default: GET)\n"
"  -b BODY         Request body\n"
"  -h NAME VALUE   Add a request header (repeatable; VALUE defaults to not-specified)\n"
"  -u USERAGENT    Override the User-Agent\n"
"  -d FILE         Download to an existing file; fail if it does not exist\n"
"  -D FILE         Download to FILE, creating it if necessary\n"
"  -Dd             Download using the remote filename\n"
"  -R              Follow redirects\n"
"  -rH             Display response headers on stderr\n"
"  -rHd FILE       Save response headers to FILE\n"
"  -v              Verbose diagnostics\n"
"  -U FILE         Upload FILE as the request body\n"
"  -t TIME         Total request timeout in milliseconds\n"
"  -T TIME         Connection timeout in milliseconds\n"
"  -mR NUM         Maximum redirects (default: unlimited when -R is used)\n"
"  -4              Force IPv4\n"
"  -6              Force IPv6\n"
"  --help          Show this help\n";
}

bool valid_method(const std::string& method) {
    static const char* methods[] = {
        "GET", "HEAD", "POST", "PUT", "DELETE", "OPTIONS", "TRACE", "PATCH"
    };
    return std::find_if(std::begin(methods), std::end(methods),
                        [&](const char* candidate) { return method == candidate; })
           != std::end(methods);
}

bool need_value(int& i, int argc, char** argv, const char* option, std::string& value) {
    if (i + 1 >= argc) {
        std::cerr << "sttp: " << option << " requires an argument\n";
        return false;
    }
    value = argv[++i];
    return true;
}

bool parse_args(int argc, char** argv, Config& cfg) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        std::string value;

        if (arg == "--help" || arg == "-?") {
            usage(std::cout);
            std::exit(0);
        } else if (arg == "-m") {
            if (!need_value(i, argc, argv, "-m", value)) return false;
            cfg.method = value;
            std::transform(cfg.method.begin(), cfg.method.end(), cfg.method.begin(),
                           [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        } else if (arg == "-b") {
            if (!need_value(i, argc, argv, "-b", cfg.body)) return false;
            cfg.body_specified = true;
        } else if (arg == "-h") {
            std::string name;
            if (!need_value(i, argc, argv, "-h", name)) return false;
            std::string header_value = "not-specified";
            if (i + 1 < argc && argv[i + 1][0] != '-') header_value = argv[++i];
            cfg.headers.emplace_back(std::move(name), std::move(header_value));
        } else if (arg == "-u") {
            if (!need_value(i, argc, argv, "-u", cfg.user_agent)) return false;
        } else if (arg == "-d" || arg == "-D") {
            if (cfg.download_option_seen) {
                std::cerr << "sttp: choose only one of -d, -D or -Dd\n";
                return false;
            }
            if (!need_value(i, argc, argv, arg.c_str(), cfg.download_file)) return false;
            cfg.download_option_seen = true;
            cfg.download_existing_only = (arg == "-d");
        } else if (arg == "-Dd") {
            if (cfg.download_option_seen) {
                std::cerr << "sttp: choose only one of -d, -D or -Dd\n";
                return false;
            }
            cfg.download_option_seen = true;
            cfg.download_remote_name = true;
        } else if (arg == "-R") {
            cfg.follow_redirects = true;
        } else if (arg == "-rH") {
            cfg.show_response_headers = true;
        } else if (arg == "-rHd") {
            if (!need_value(i, argc, argv, "-rHd", cfg.response_headers_file)) return false;
        } else if (arg == "-v") {
            cfg.verbose = true;
        } else if (arg == "-U") {
            if (!need_value(i, argc, argv, "-U", cfg.upload_file)) return false;
        } else if (arg == "-t") {
            if (!need_value(i, argc, argv, "-t", value) || !parse_long(value.c_str(), cfg.timeout_ms)) {
                std::cerr << "sttp: -t expects a non-negative integer number of milliseconds\n";
                return false;
            }
        } else if (arg == "-T") {
            if (!need_value(i, argc, argv, "-T", value) || !parse_long(value.c_str(), cfg.connect_timeout_ms)) {
                std::cerr << "sttp: -T expects a non-negative integer number of milliseconds\n";
                return false;
            }
        } else if (arg == "-mR") {
            if (!need_value(i, argc, argv, "-mR", value) || !parse_long(value.c_str(), cfg.max_redirects)) {
                std::cerr << "sttp: -mR expects a non-negative integer\n";
                return false;
            }
        } else if (arg == "-4") {
            cfg.force_ipv4 = true;
        } else if (arg == "-6") {
            cfg.force_ipv6 = true;
        } else if (arg.size() > 1 && arg[0] == '-') {
            std::cerr << "sttp: unknown option: " << arg << "\n";
            return false;
        } else if (cfg.site.empty()) {
            cfg.site = arg;
        } else {
            std::cerr << "sttp: unexpected extra argument: " << arg << "\n";
            return false;
        }
    }

    if (cfg.site.empty()) {
        std::cerr << "sttp: missing SITE\n";
        return false;
    }
    if (!valid_method(cfg.method)) {
        std::cerr << "sttp: unsupported method: " << cfg.method << "\n";
        return false;
    }
    if (cfg.force_ipv4 && cfg.force_ipv6) {
        std::cerr << "sttp: -4 and -6 cannot be used together\n";
        return false;
    }
    if (!cfg.upload_file.empty() && cfg.body_specified) {
        std::cerr << "sttp: -b and -U cannot be used together\n";
        return false;
    }
    if ((cfg.method == "HEAD") && (!cfg.body.empty() || !cfg.upload_file.empty())) {
        std::cerr << "sttp: HEAD requests cannot use -b or -U\n";
        return false;
    }
    return true;
}

std::string os_name() {
#if defined(__linux__)
    return "Linux";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__FreeBSD__)
    return "FreeBSD";
#elif defined(__OpenBSD__)
    return "OpenBSD";
#elif defined(__NetBSD__)
    return "NetBSD";
#elif defined(__DragonFly__)
    return "DragonFlyBSD";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "UnknownOS";
#endif
}

std::string default_user_agent() {
    const curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);
    const char* version = (info && info->version) ? info->version : "unknown";
    return "sttp/all (" + os_name() + "; libcurl/" + version + ")";
}

std::string decode_percent(const std::string& input) {
    std::string output;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%' && i + 2 < input.size()) {
            auto hex = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            const int hi = hex(input[i + 1]);
            const int lo = hex(input[i + 2]);
            if (hi >= 0 && lo >= 0) {
                output.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        output.push_back(input[i]);
    }
    return output;
}

std::string safe_basename(std::string name) {
    const auto slash = name.find_last_of("/\\");
    if (slash != std::string::npos) name = name.substr(slash + 1);
    name = trim(name);
    if (name.empty() || name == "." || name == "..") return "download";
    for (char& c : name) {
        const unsigned char u = static_cast<unsigned char>(c);
        if (u < 32 || c == 127 || c == ':' || c == '/' || c == '\\') c = '_';
    }
    return name;
}

std::string filename_from_url(const std::string& url) {
    std::string path;
    CURLU* parsed = curl_url();
    if (parsed) {
        if (curl_url_set(parsed, CURLUPART_URL, url.c_str(), 0) == CURLUE_OK) {
            char* raw = nullptr;
            if (curl_url_get(parsed, CURLUPART_PATH, &raw, 0) == CURLUE_OK && raw) {
                path = raw;
                curl_free(raw);
            }
        }
        curl_url_cleanup(parsed);
    }
    if (path.empty() || path.back() == '/') return "index.html";
    return safe_basename(decode_percent(path));
}

std::string disposition_filename(const std::string& header) {
    const std::string value = header.substr(header.find(':') + 1);
    const std::string low = lower(value);
    std::size_t pos = low.find("filename*=");
    if (pos != std::string::npos) {
        std::string raw = trim(value.substr(pos + 10));
        const auto quote = raw.find('\'');
        if (quote != std::string::npos) {
            const auto second = raw.find('\'', quote + 1);
            if (second != std::string::npos) raw = raw.substr(second + 1);
        }
        const auto semi = raw.find(';');
        if (semi != std::string::npos) raw.resize(semi);
        if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') raw = raw.substr(1, raw.size() - 2);
        return safe_basename(decode_percent(raw));
    }
    pos = low.find("filename=");
    if (pos == std::string::npos) return {};
    std::string raw = trim(value.substr(pos + 9));
    const auto semi = raw.find(';');
    if (semi != std::string::npos) raw.resize(semi);
    raw = trim(raw);
    if (raw.size() >= 2 && raw.front() == '"' && raw.back() == '"') raw = raw.substr(1, raw.size() - 2);
    return safe_basename(raw);
}

bool open_remote_output(Transfer* transfer) {
    if (transfer->output_opened) return transfer->output != nullptr;
    if (!transfer->config->download_remote_name) return false;
    const std::string filename = transfer->remote_filename.empty()
        ? filename_from_url(transfer->config->site) : transfer->remote_filename;
    transfer->output = std::fopen(filename.c_str(), "wb");
    if (!transfer->output) {
        std::cerr << "sttp: cannot create download file '" << filename << "': " << std::strerror(errno) << "\n";
        transfer->output_error = true;
        return false;
    }
    transfer->output_opened = true;
    if (transfer->config->verbose) std::cerr << "sttp: writing response to " << filename << "\n";
    return true;
}

size_t write_callback(char* data, size_t size, size_t count, void* userdata) {
    auto* transfer = static_cast<Transfer*>(userdata);
    const size_t bytes = size * count;
    if (transfer->config->download_remote_name && !open_remote_output(transfer)) return 0;
    FILE* destination = transfer->output ? transfer->output : stdout;
    const size_t written = std::fwrite(data, 1, bytes, destination);
    transfer->downloaded += static_cast<curl_off_t>(written);
    if (written != bytes) {
        transfer->output_error = true;
        return 0;
    }
    return written;
}

size_t header_callback(char* data, size_t size, size_t count, void* userdata) {
    auto* transfer = static_cast<Transfer*>(userdata);
    const size_t bytes = size * count;
    const std::string line(data, bytes);
    const auto colon = line.find(':');
    if (colon != std::string::npos) {
        const std::string name = lower(trim(line.substr(0, colon)));
        const std::string value = trim(line.substr(colon + 1));
        if (name == "content-type") transfer->content_type = value;
        if (name == "server") transfer->server = value;
        if (name == "content-disposition" && transfer->config->download_remote_name) {
            const std::string filename = disposition_filename(line);
            if (!filename.empty()) transfer->remote_filename = filename;
        }
    }

    if (transfer->headers_output) {
        if (std::fwrite(data, 1, bytes, transfer->headers_output) != bytes) return 0;
    }
    if (transfer->config->show_response_headers || transfer->config->verbose) {
        if (std::fwrite(data, 1, bytes, stderr) != bytes) return 0;
    }
    return bytes;
}

size_t read_callback(char* buffer, size_t size, size_t count, void* userdata) {
    auto* transfer = static_cast<Transfer*>(userdata);
    if (!transfer->upload) return CURL_READFUNC_ABORT;
    const size_t capacity = size * count;
    const size_t read = std::fread(buffer, 1, capacity, transfer->upload);
    if (read == 0 && std::ferror(transfer->upload)) return CURL_READFUNC_ABORT;
    transfer->uploaded += static_cast<curl_off_t>(read);
    return read;
}

void close_file(FILE*& file) {
    if (file) {
        std::fclose(file);
        file = nullptr;
    }
}

int run(const Config& cfg) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "sttp: unable to initialize libcurl\n";
        return 2;
    }

    Transfer transfer;
    transfer.config = const_cast<Config*>(&cfg);
    transfer.remote_filename = cfg.download_remote_name ? filename_from_url(cfg.site) : "";

    if (!cfg.download_file.empty()) {
        if (cfg.download_existing_only) {
            FILE* check = std::fopen(cfg.download_file.c_str(), "rb");
            if (!check) {
                std::cerr << "sttp: download target does not exist: " << cfg.download_file << "\n";
                curl_easy_cleanup(curl);
                return 2;
            }
            std::fclose(check);
        }
        transfer.output = std::fopen(cfg.download_file.c_str(), "wb");
        if (!transfer.output) {
            std::cerr << "sttp: cannot open download target '" << cfg.download_file << "': " << std::strerror(errno) << "\n";
            curl_easy_cleanup(curl);
            return 2;
        }
        transfer.output_opened = true;
    }

    if (!cfg.response_headers_file.empty()) {
        transfer.headers_output = std::fopen(cfg.response_headers_file.c_str(), "wb");
        if (!transfer.headers_output) {
            std::cerr << "sttp: cannot open response-header file '" << cfg.response_headers_file << "': " << std::strerror(errno) << "\n";
            close_file(transfer.output);
            curl_easy_cleanup(curl);
            return 2;
        }
    }

    curl_slist* request_headers = nullptr;
    for (const auto& header : cfg.headers) {
        const std::string line = header.first + ": " + header.second;
        curl_slist* appended = curl_slist_append(request_headers, line.c_str());
        if (!appended) {
            std::cerr << "sttp: unable to allocate request headers\n";
            curl_slist_free_all(request_headers);
            close_file(transfer.output);
            close_file(transfer.headers_output);
            curl_easy_cleanup(curl);
            return 2;
        }
        request_headers = appended;
    }

    if (!cfg.upload_file.empty()) {
        transfer.upload = std::fopen(cfg.upload_file.c_str(), "rb");
        if (!transfer.upload) {
            std::cerr << "sttp: cannot open upload file '" << cfg.upload_file << "': " << std::strerror(errno) << "\n";
            curl_slist_free_all(request_headers);
            close_file(transfer.output);
            close_file(transfer.headers_output);
            curl_easy_cleanup(curl);
            return 2;
        }
    }

    const std::string user_agent = cfg.user_agent.empty() ? default_user_agent() : cfg.user_agent;
    curl_easy_setopt(curl, CURLOPT_URL, cfg.site.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &transfer);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &transfer);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    if (request_headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request_headers);
    if (cfg.timeout_ms > 0) curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, cfg.timeout_ms);
    if (cfg.connect_timeout_ms > 0) curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, cfg.connect_timeout_ms);
    if (cfg.force_ipv4) curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    if (cfg.force_ipv6) curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
    if (cfg.follow_redirects) {
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, cfg.max_redirects);
        curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS_STR, "http,https");
    }
    if (cfg.verbose) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    if (cfg.method == "HEAD") {
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "HEAD");
    } else if (!cfg.upload_file.empty()) {
        if (std::fseek(transfer.upload, 0, SEEK_END) != 0) {
            std::cerr << "sttp: unable to determine upload file size\n";
            close_file(transfer.upload);
            curl_slist_free_all(request_headers);
            close_file(transfer.output);
            close_file(transfer.headers_output);
            curl_easy_cleanup(curl);
            return 2;
        }
        const long long file_size = static_cast<long long>(std::ftell(transfer.upload));
        std::rewind(transfer.upload);
        if (file_size < 0) {
            std::cerr << "sttp: unable to determine upload file size\n";
            close_file(transfer.upload);
            curl_slist_free_all(request_headers);
            close_file(transfer.output);
            close_file(transfer.headers_output);
            curl_easy_cleanup(curl);
            return 2;
        }
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &transfer);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(file_size));
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, cfg.method.c_str());
    } else if (cfg.body_specified) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cfg.body.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(cfg.body.size()));
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, cfg.method.c_str());
    } else if (cfg.method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    } else if (cfg.method == "GET") {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    } else {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, cfg.method.c_str());
    }

    const CURLcode result = curl_easy_perform(curl);

    long status = 0;
    double total_time = 0.0;
    char* effective_url = nullptr;
    curl_off_t download_size = transfer.downloaded;
    curl_off_t upload_size = transfer.uploaded;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
    curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effective_url);
    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &download_size);
    curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &upload_size);

    if (cfg.download_remote_name && !transfer.output_opened && result == CURLE_OK) {
        open_remote_output(&transfer);
    }

    if (cfg.verbose) {
        char* content_type = nullptr;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
        std::cerr << "\nsttp: transfer summary\n"
                  << "  URL: " << (effective_url ? effective_url : cfg.site) << "\n"
                  << "  Method: " << cfg.method << "\n"
                  << "  HTTP status: " << status << "\n"
                  << "  Content-Type: " << (content_type ? content_type : "unknown") << "\n"
                  << "  Server: " << (transfer.server.empty() ? "unknown" : transfer.server) << "\n"
                  << "  Downloaded: " << download_size << " bytes\n"
                  << "  Uploaded: " << upload_size << " bytes\n"
                  << "  Total time: " << total_time << " seconds\n";
    }

    if (transfer.output) std::fflush(transfer.output);
    if (transfer.headers_output) std::fflush(transfer.headers_output);
    const bool output_error = transfer.output_error || (transfer.output && std::ferror(transfer.output));

    close_file(transfer.upload);
    close_file(transfer.output);
    close_file(transfer.headers_output);
    curl_slist_free_all(request_headers);
    curl_easy_cleanup(curl);

    if (output_error) {
        std::cerr << "sttp: error writing output\n";
        return 2;
    }
    if (result != CURLE_OK) {
        std::cerr << "sttp: " << curl_easy_strerror(result) << "\n";
        return (result == CURLE_OPERATION_TIMEDOUT) ? 28 : 3;
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    Config config;
    if (!parse_args(argc, argv, config)) {
        std::cerr << "Try 'sttp --help' for usage.\n";
        return 1;
    }

    const CURLcode init = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (init != CURLE_OK) {
        std::cerr << "sttp: libcurl initialization failed: " << curl_easy_strerror(init) << "\n";
        return 2;
    }

    const int result = run(config);
    curl_global_cleanup();
    return result;
}
