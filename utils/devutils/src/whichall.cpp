#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

void print_help() {
    std::cout << "whichall [OPTIONS] <binary>\n\n"
              << "Options:\n"
              << "    -h      -- Display help.\n"
              << "    -j      -- Output JSON.\n"
              << "    -x      -- Output XML.\n"
              << "    -y      -- Output YAML.\n";
}

struct ExecResult {
    std::string path;
    std::string mod;
    std::string user;
};

std::string get_file_mod_time(const std::string& filepath) {
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        return "";
    }
    time_t mtime = st.st_mtime;
    struct tm* gmt = gmtime(&mtime);
    if (!gmt) {
        return "";
    }
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmt);
    return std::string(buf);
}

std::string get_file_owner(const std::string& filepath) {
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        return "";
    }
#if defined(_WIN32) || defined(_WIN64)
    return "";
#else
    struct passwd* pw = getpwuid(st.st_uid);
    if (pw && pw->pw_name) {
        return std::string(pw->pw_name);
    }
    return "";
#endif
}

std::string get_absolute_path(const std::string& path) {
    char resolved[PATH_MAX];
    if (realpath(path.c_str(), resolved)) {
        return std::string(resolved);
    }
    return path;
}

bool is_executable(const std::string& filepath) {
    struct stat st;
    if (stat(filepath.c_str(), &st) != 0) {
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        return false;
    }
#if defined(_WIN32) || defined(_WIN64)
    return true;
#else
    return access(filepath.c_str(), X_OK) == 0;
#endif
}

std::vector<std::string> split_path_env(const std::string& path_env) {
    std::vector<std::string> paths;
    std::stringstream ss(path_env);
    std::string item;
#if defined(_WIN32) || defined(_WIN64)
    char delim = ';';
#else
    char delim = ':';
#endif
    while (std::getline(ss, item, delim)) {
        if (!item.empty()) {
            paths.push_back(item);
        }
    }
    return paths;
}

int main(int argc, char* argv[]) {
    bool json_out = false;
    bool xml_out = false;
    bool yaml_out = false;
    std::string target = "";
    bool dashdash = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (!dashdash && arg == "--") {
            dashdash = true;
            continue;
        }
        if (!dashdash && arg.rfind("-", 0) == 0 && arg != "-") {
            if (arg == "-h" || arg == "--help") {
                print_help();
                return 0;
            } else if (arg == "-j") {
                json_out = true;
            } else if (arg == "-x") {
                xml_out = true;
            } else if (arg == "-y") {
                yaml_out = true;
            } else {
                std::cerr << "Error: Unknown option " << arg << "\n";
                return 2;
            }
        } else {
            if (!target.empty()) {
                std::cerr << "Error: Multiple target binaries specified.\n";
                return 2;
            }
            target = arg;
        }
    }

    if (target.empty()) {
        std::cerr << "Error: No target binary specified.\n";
        return 2;
    }

    int format_count = (json_out ? 1 : 0) + (xml_out ? 1 : 0) + (yaml_out ? 1 : 0);
    if (format_count > 1) {
        std::cerr << "Error: Conflicting output format options specified.\n";
        return 2;
    }

    const char* path_env_cstr = std::getenv("PATH");
    std::string path_env = path_env_cstr ? path_env_cstr : "";
    std::vector<std::string> dirs = split_path_env(path_env);

    std::vector<ExecResult> results;
    std::set<std::string> seen_paths;

    char cwd_buf[PATH_MAX];
    std::string cwd = getcwd(cwd_buf, sizeof(cwd_buf)) ? cwd_buf : "";

#if defined(_WIN32) || defined(_WIN64)
    std::vector<std::string> extensions = {""};
    const char* pathext = std::getenv("PATHEXT");
    if (pathext) {
        std::stringstream pext_ss(pathext);
        std::string ext;
        while (std::getline(pext_ss, ext, ';')) {
            if (!ext.empty()) {
                extensions.push_back(ext);
            }
        }
    }
#else
    std::vector<std::string> extensions = {"%"};
#endif

    for (const auto& dir : dirs) {
        std::string resolved_dir = dir;
        if (!resolved_dir.empty() && resolved_dir[0] != '/') {
#if !defined(_WIN32) && !defined(_WIN64)
            if (cwd.empty() || resolved_dir[0] != '~') {
            }
#endif
        }

        std::string full_path = dir;
        if (!full_path.empty() && full_path.back() != '/' && full_path.back() != '\\') {
            full_path += "/";
        }
        full_path += target;

#if defined(_WIN32) || defined(_WIN64)
        for (const auto& ext : extensions) {
            std::string test_path = full_path + ext;
            if (is_executable(test_path)) {
                std::string abs_p = get_absolute_path(test_path);
                if (seen_paths.find(abs_p) == seen_paths.end()) {
                    seen_paths.insert(abs_p);
                    std::string mod = get_file_mod_time(test_path);
                    std::string user = get_file_owner(test_path);
                    results.push_back({abs_p, mod, user.empty() ? "null" : user});
                }
            }
        }
#else
        if (is_executable(full_path)) {
            std::string abs_p = get_absolute_path(full_path);
            if (seen_paths.find(abs_p) == seen_paths.end()) {
                seen_paths.insert(abs_p);
                std::string mod = get_file_mod_time(full_path);
                std::string user = get_file_owner(full_path);
                results.push_back({abs_p, mod, user.empty() ? "null" : user});
            }
        }
#endif
    }

    if (json_out) {
        std::cout << "{\n  \"executables\": [\n";
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << "    {\n"
                      << "      \"path\": \"" << results[i].path << "\",\n"
                      << "      \"mod\": \"" << results[i].mod << "\",\n"
                      << "      \"user\": " << (results[i].user == "null" ? "null" : "\"" + results[i].user + "\"") << "\n"
                      << "    }" << (i + 1 < results.size() ? "," : "") << "\n";
        }
        std::cout << "  ]\n}\n";
    } else if (yaml_out) {
        std::cout << "executables:\n";
        for (const auto& res : results) {
            std::cout << "  - path: " << res.path << "\n"
                      << "    mod: \"" << res.mod << "\"\n"
                      << "    user: " << res.user << "\n";
        }
    } else if (xml_out) {
        std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<executables>\n";
        for (const auto& res : results) {
            std::cout << "  <executable>\n"
                      << "    <path>" << res.path << "</path>\n"
                      << "    <mod>" << res.mod << "</mod>\n"
                      << "    <user>" << res.user << "</user>\n"
                      << "  </executable>\n";
        }
        std::cout << "</executables>\n";
    } else {
        for (const auto& res : results) {
            std::cout << res.path << ";" << res.mod << ";" << res.user << "\n";
        }
    }

    return results.empty() ? 1 : 0;
}
