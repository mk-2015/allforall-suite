#include <pkg.hpp>
#include <pkg_db.hpp>
#include <deps.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <random>

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

std::vector<std::string> load_repo_urls(); // Declared elsewhere or in headers

// Helper structure to represent potential upgrade candidates across repositories
struct RepoUpgradeMatch {
    std::string repo_url;
    std::string manifest_path;
    BoxConfig config;
};

struct UpgradeCandidate {
    std::string name;
    std::string current_version;
    std::string latest_version;
    BoxConfig config;
};

// Simple version comparison helper
bool is_newer_version(const std::string& current, const std::string& latest) {
    if (current == latest) return false;
    return !latest.empty();
}

void hpm_upgrade_packages() {
    hpm_info("Checking for package updates across repositories...\n");

    // 1. Get all installed packages from SQLite DB (Expected format: "name:version")
    std::vector<std::string> installed_pkgs = db_list_installed_packages();
    if (installed_pkgs.empty()) {
        std::printf("No packages installed to upgrade.\n");
        return;
    }

    // 2. Load repositories
    std::vector<std::string> repos = load_repo_urls();
    if (repos.empty()) {
        hpm_fail(1, "Error: No repositories configured in ~/.hpm/.config.yml\n");
    }

    fs::path tmp_dir = get_temp_directory();
    fs::create_directories(tmp_dir);

    std::vector<UpgradeCandidate> upgrades;

    // 3. Scan repositories for updates and handle conflicts interactively
    for (const auto& local_pkg_entry : installed_pkgs) {
        std::string pkg_name;
        std::string current_version;
        size_t delimiter = local_pkg_entry.find(':');
        
        if (delimiter != std::string::npos) {
            pkg_name = local_pkg_entry.substr(0, delimiter);
            current_version = local_pkg_entry.substr(delimiter + 1);
        } else {
            pkg_name = local_pkg_entry;
            current_version = "unknown";
        }

        std::vector<RepoUpgradeMatch> valid_matches;

        for (const auto& repo : repos) {
            std::string manifest_url = repo + "/" + pkg_name + "/latest";
            std::string fetched_manifest_path = (tmp_dir / (pkg_name + "-latest-" + std::to_string(rand()) + ".yaml")).string();

            if (download_file(manifest_url, fetched_manifest_path)) {
                BoxConfig temp_config;
                if (parse_box_yaml(fetched_manifest_path, temp_config)) {
                    if (is_newer_version(current_version, temp_config.version)) {
                        valid_matches.push_back({repo, fetched_manifest_path, temp_config});
                    } else {
                        fs::remove(fetched_manifest_path);
                    }
                } else {
                    fs::remove(fetched_manifest_path);
                }
            }
        }

        if (valid_matches.empty()) {
            continue; // No newer versions found for this package
        }

        BoxConfig selected_config;
        std::string chosen_repo;

        if (valid_matches.size() == 1) {
            chosen_repo = valid_matches[0].repo_url;
            selected_config = valid_matches[0].config;
            fs::remove(valid_matches[0].manifest_path);
        } else {
            printf("\n[SECURITY CONFLICT] Multiple repositories offer a newer version for '%s':\n", pkg_name.c_str());
            for (size_t i = 0; i < valid_matches.size(); ++i) {
                printf("  [%zu] Repo: %s\n      -> Version: %s (Maintainer: %s)\n", 
                    i + 1, 
                    valid_matches[i].repo_url.c_str(), 
                    valid_matches[i].config.version.c_str(),
                    valid_matches[i].config.maintainer.c_str());
            }
            printf("Select repository number to upgrade from: ");
            
            size_t choice = 1;
            if (!(std::cin >> choice) || choice < 1 || choice > valid_matches.size()) {
                printf("Invalid selection. Skipping upgrade for '%s'.\n", pkg_name.c_str());
                for (const auto& m : valid_matches) fs::remove(m.manifest_path);
                continue;
            }
            
            chosen_repo = valid_matches[choice - 1].repo_url;
            selected_config = valid_matches[choice - 1].config;
            fs::remove(valid_matches[choice - 1].manifest_path);

            // Cleanup unselected manifests
            for (size_t i = 0; i < valid_matches.size(); ++i) {
                if (i != (choice - 1)) {
                    fs::remove(valid_matches[i].manifest_path);
                }
            }
        }

        upgrades.push_back({
            pkg_name,
            current_version,
            selected_config.version,
            selected_config
        });
    }

    if (upgrades.empty()) {
        std::printf("All packages are already up to date!\n");
        return;
    }

    std::printf("Found %zu package(s) to upgrade:\n", upgrades.size());
    for (const auto& up : upgrades) {
        std::printf("  - %s: %s -> %s\n", up.name.c_str(), up.current_version.c_str(), up.latest_version.c_str());
    }

    // 4. Download, Verify, and Build loop for each upgrade candidate
    for (const auto& up : upgrades) {
        printf("===> Upgrading package: %s to v%s\n", up.name.c_str(), up.latest_version.c_str());

        fs::path build_dir = tmp_dir / ("build-upgrade-" + up.name);
        fs::create_directories(build_dir);

        bool download_and_extract_success = true;

        for (const auto& named_pkg : up.config.packages) {
            std::string archive_path = (build_dir / (named_pkg.name + ".zip")).string();
            std::string sig_path = archive_path + ".sig";
            
            printf("===> Downloading %s from %s...\n", named_pkg.name.c_str(), named_pkg.url.c_str());

            if (!download_file(named_pkg.url, archive_path)) {
                hpm_fail((int)NULL, "Failed to download package archive '%s'\n", named_pkg.name.c_str());
                download_and_extract_success = false;
                break;
            }

            std::string sig_url = named_pkg.url + ".sig";
            printf("===> Downloading package signature for %s...\n", named_pkg.name.c_str());
            
            if (!download_file(sig_url, sig_path)) {
                hpm_fail((int)NULL, "Security Error: Failed to download signature file for '%s'. Aborting upgrade.\n", named_pkg.name.c_str());
                download_and_extract_success = false;
                break;
            }

            // Read archive bytes into memory
            std::vector<unsigned char> file_data;
            {
                std::ifstream file(archive_path, std::ios::binary | std::ios::ate);
                if (!file.is_open()) {
                    hpm_fail((int)NULL, "Error opening upgrade archive file.\n");
                    download_and_extract_success = false;
                    break;
                }
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                file_data.resize(size);
                if (!file.read((char*)file_data.data(), size)) {
                    hpm_fail((int)NULL, "Error reading upgrade archive into memory.\n");
                    download_and_extract_success = false;
                    break;
                }
            }

            // Read signature bytes into memory
            std::vector<unsigned char> signature_data;
            {
                std::ifstream sig_file(sig_path, std::ios::binary | std::ios::ate);
                if (!sig_file.is_open()) {
                    hpm_fail((int)NULL, "Error opening upgrade signature file.\n");
                    download_and_extract_success = false;
                    break;
                }
                std::streamsize sig_size = sig_file.tellg();
                sig_file.seekg(0, std::ios::beg);
                signature_data.resize(sig_size);
                if (!sig_file.read((char*)signature_data.data(), sig_size)) {
                    hpm_fail((int)NULL, "Error reading upgrade signature file into memory.\n");
                    download_and_extract_success = false;
                    break;
                }
            }

            std::string maintainer = up.config.maintainer.empty() ? "official" : up.config.maintainer;
            printf("===> Verifying Ed25519 signature for %s (Key: %s)...\n", named_pkg.name.c_str(), maintainer.c_str());
            
            if (!verify_with_maintainer_key(maintainer, file_data, signature_data)) {
                hpm_fail(1, "CRITICAL SECURITY ERROR: Signature verification failed for package '%s'! Aborting upgrade.\n", named_pkg.name.c_str());
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
            continue;
        }

        // 5. Execute Build Commands
        printf("===> Running build commands for %s...\n", up.name.c_str());
        bool build_success = true;

        for (const auto& cmd : up.config.commands) {
            std::string exec_cmd = "cd " + build_dir.string() + " && " + cmd;
            int res = std::system(exec_cmd.c_str());
            if (res != 0) {
                hpm_fail((int)NULL, "Command failed with code %d: %s\n", res, cmd.c_str());
                build_success = false;
                break;
            }
        }

        // 6. Update SQLite Database State
        if (build_success) {
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
            }

            db_register_package(up.config.name, up.config.version, installed_files);
            printf("===> Successfully upgraded %s to v%s\n", up.config.name.c_str(), up.config.version.c_str());
        }

        fs::remove_all(build_dir);
    }
}