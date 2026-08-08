#include <pkg.hpp>
#include <deps.hpp>
#include <pkg_db.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <cstdio>

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

std::vector<std::string> load_repo_urls() {
    const char* home = std::getenv("HOME");
    if (!home) home = std::getenv("USERPROFILE");
    if (!home) return {};

    fs::path config_path = fs::path(home) / ".hpm" / ".config.yml";
    return parse_hpm_config_repos(config_path.string());
}

bool hpm_install_packages(const std::vector<std::string>& pkglist) {
    std::vector<std::string> repos = load_repo_urls();
    if (repos.empty()) {
        hpm_fail(1, "No repositories configured in ~/.hpm/.config.yml\n");
        return false;
    }

    fs::path tmp_dir = get_temp_directory();
    fs::create_directories(tmp_dir);

    for (const auto& pkg : pkglist) {
        PackageSpec spec = parse_package_spec(pkg);
        
        printf("===> Processing package: %s\n", pkg.c_str());

        if (db_is_package_installed(pkg)) {
            printf("===> Package '%s' is already installed. Skipping.\n", pkg.c_str());
            continue;
        }

        std::string manifest_path = (tmp_dir / (pkg + ".yaml")).string();
        bool downloaded = false;

        for (const auto& repo : repos) {
            std::string manifest_url = repo + "/" + spec.name + "/" + spec.version;
            printf("===> Fetching manifest from %s\n", manifest_url.c_str());

            if (download_file(manifest_url, manifest_path)) {
                downloaded = true;
                break;
            }
        }

        if (!downloaded) {
            hpm_fail((int)NULL, "Failed to fetch manifest for package '%s' from any repository.\n", pkg.c_str());
            continue;
        }

        BoxConfig config;
        if (!parse_box_yaml(manifest_path, config)) {
            hpm_fail((int)NULL, "Failed to parse box manifest for: %s\n", pkg.c_str());
            fs::remove(manifest_path);
            continue;
        }

        if (!config.dependencies.empty()) {
            printf("===> Resolving dependencies for %s...\n", pkg.c_str());
            std::vector<std::string> dep_names;
            for (const auto& dep : config.dependencies) {
                dep_names.push_back(dep.name);
            }
            hpm_install_packages(dep_names);
        }

        fs::path build_dir = tmp_dir / ("build-" + pkg);
        fs::create_directories(build_dir);

        bool download_and_extract_success = true;

        for (const auto& named_pkg : config.packages) {
            std::string archive_path = (build_dir / (named_pkg.name + ".zip")).string();
            printf("===> Downloading %s from %s...\n", named_pkg.name.c_str(), named_pkg.url.c_str());

            if (!download_file(named_pkg.url, archive_path)) {
                hpm_fail((int)NULL, "Failed to download package archive '%s' from %s\n", named_pkg.name.c_str(), named_pkg.url.c_str());
                download_and_extract_success = false;
                break;
            }

            printf("===> Extracting archive %s...\n", named_pkg.name.c_str());
            if (!extract_zip(archive_path, build_dir.string())) {
                hpm_fail((int)NULL, "Failed to extract archive: %s\n", archive_path.c_str());
                download_and_extract_success = false;
                break;
            }
        }

        if (!download_and_extract_success) {
            fs::remove_all(build_dir);
            fs::remove(manifest_path);
            continue;
        }

        printf("===> Running build commands for %s...\n", pkg.c_str());
        bool build_success = true;

        for (const auto& cmd : config.commands) {
            std::string exec_cmd = "cd " + build_dir.string() + " && " + cmd;
            int res = std::system(exec_cmd.c_str());
            if (res != 0) {
                hpm_fail((int)NULL, "Command failed with code %d: %s\n", res, cmd.c_str());
                build_success = false;
                break;
            }
        }

        if (build_success) {
            // FIX 2: Check for install_manifest.txt and pass file paths into SQLite
            fs::path manifest_txt = build_dir / "install_manifest.txt";
            std::vector<std::string> installed_files;

            if (fs::exists(manifest_txt)) {
                std::ifstream file(manifest_txt);
                std::string line;
                while (std::getline(file, line)) {
                    if (!line.empty()) {
                        installed_files.push_back(line);
                    }
                }
                printf("===> Tracked %zu installed files from install_manifest.txt\n", installed_files.size());
            } else {
                printf("===> Warning: No install_manifest.txt produced during build.\n");
            }

            db_register_package(config.name, config.version, installed_files);
            printf("===> Successfully installed %s v%s\n", config.name.c_str(), config.version.c_str());
        }

        fs::remove_all(build_dir);
        fs::remove(manifest_path);
    }

    return true;
}