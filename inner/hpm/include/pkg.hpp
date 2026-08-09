#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

struct Dependency {
    std::string name;
    std::string constraint;
};

struct NamedPackage {
    std::string name;
    std::string url;
};

struct BoxConfig {
    std::string name;
    std::string version;
    std::string maintainer;
    std::vector<Dependency> dependencies;
    std::vector<NamedPackage> packages;
    std::vector<std::string> commands;
};

struct RemotePackageInfo {
    std::string name;
    std::string repo_url;
};

struct PackageSpec {
    std::string name;
    std::string version;
};

void hpm_fail(int autoexit, std::string str, ...); // Error

inline bool is_valid_pkg_identifier(const std::string& str) {
    if (str.empty()) return false;
    return std::all_of(str.begin(), str.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '+';
    });
}

inline PackageSpec parse_package_spec(const std::string& input) {
    PackageSpec spec;
    size_t pos = input.find(':');
    
    if (pos != std::string::npos) {
        spec.name = input.substr(0, pos);
        spec.version = input.substr(pos + 1);
        if (spec.version.empty()) {
            spec.version = "latest";
        }
    } else {
        spec.name = input;
        spec.version = "latest";
    }

    if (!is_valid_pkg_identifier(spec.name) || !is_valid_pkg_identifier(spec.version)) {
        hpm_fail(1, "Invalid package specifier '%s'. Package names and versions may only contain alphanumeric characters, '-', '_', '.', and '+'.\n", input.c_str());
        spec.name = "";
        spec.version = "";
    }
    
    return spec;
}

void hpm_ok(std::string str, ...);
void hpm_info(std::string str, ...);
int hpm_init();

bool hpm_install_packages(const std::vector<std::string>& pkglist);
int hpm_uninstall_package(const std::vector<std::string>& pkglist, bool force = false);
std::vector<std::string> load_repo_urls();
std::vector<RemotePackageInfo> search_remote_repositories(const std::string& pattern_str);
void hpm_upgrade_packages();

bool parse_box_yaml(const std::string& filepath, BoxConfig& out_config);
std::vector<std::string> parse_hpm_config_repos(const std::string& filepath);

bool keyring_add_maintainer(std::string name, fs::path keyfile);
bool keyring_remove_maintainer(std::string name);
std::vector<std::string> keyring_list_maintainers();
bool keyring_generate_maintainer(std::string name, std::string email, fs::path private_keyring);
bool keyring_extract_maintainer(std::string name, fs::path output_path);
bool keyring_sign_archive(fs::path private_key_path, fs::path target_file_path, fs::path output_sig_path);