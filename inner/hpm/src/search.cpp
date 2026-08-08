#include <pkg.hpp>
#include <deps.hpp>
#include <yaml.h>
#include <regex>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

static fs::path get_temp_directory() {
#if defined(_WIN32) || defined(_WIN64)
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = std::getenv("TMP");
    if (!tmp) return fs::path("C:\\Temp\\hpm");
    return fs::path(tmp) / "hpm";
#else
    const char* tmp = std::getenv("TMPDIR");
    if (!tmp) return fs::path("/tmp/hpm");
    return fs::path(tmp) / "hpm";
#endif
}

std::vector<RemotePackageInfo> search_remote_repositories(const std::string& pattern_str) {
    std::vector<RemotePackageInfo> results;

    std::regex search_pattern;
    try {
        search_pattern = std::regex(pattern_str, std::regex::icase | std::regex::ECMAScript);
    } catch (const std::regex_error& e) {
        hpm_fail((int)NULL, "Invalid regex pattern '%s': %s\n", pattern_str.c_str(), e.what());
        return results;
    }

    std::vector<std::string> repos = load_repo_urls();
    if (repos.empty()) {
        hpm_fail((int)NULL, "No repositories configured in ~/.hpm/.config.yml\n");
        return results;
    }

    fs::path tmp_dir = get_temp_directory();
    fs::create_directories(tmp_dir);

    for (const auto& repo : repos) {
        std::string index_url = repo + "/index.yaml";
        std::string index_path = (tmp_dir / "repo_index.yaml").string();

        if (!download_file(index_url, index_path)) {
            continue;
        }

        FILE* file = std::fopen(index_path.c_str(), "rb");
        if (!file) {
            fs::remove(index_path);
            continue;
        }

        yaml_parser_t parser;
        yaml_event_t event;

        if (!yaml_parser_initialize(&parser)) {
            std::fclose(file);
            fs::remove(index_path);
            continue;
        }

        yaml_parser_set_input_file(&parser, file);

        bool is_key_name = false;

        while (true) {
            if (!yaml_parser_parse(&parser, &event)) break;

            if (event.type == YAML_SCALAR_EVENT) {
                std::string value(reinterpret_cast<char*>(event.data.scalar.value));

                if (is_key_name) {
                    if (std::regex_search(value, search_pattern)) {
                        results.push_back({value, repo});
                    }
                    is_key_name = false;
                } else if (value == "name") {
                    is_key_name = true;
                }
            } else if (event.type == YAML_STREAM_END_EVENT) {
                yaml_event_delete(&event);
                break;
            }

            yaml_event_delete(&event);
        }

        yaml_parser_delete(&parser);
        std::fclose(file);
        fs::remove(index_path);
    }

    return results;
}