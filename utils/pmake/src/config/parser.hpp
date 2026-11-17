#pragma once

#include <string>
#include <vector>
#include <optional>
#include <toml++/toml.hpp>

namespace pmake::config {

struct PackageConfig {
    std::string name;
    std::string version;
    int release = 1;
    std::string description;
    std::string architecture = "x86_64";
    std::vector<std::string> license;
};

struct BuildConfig {
    int jobs = 4;
    std::string directory = "build";
    std::string output = "pkg";
};

struct SourceConfig {
    std::string id;
    std::string name;
    std::string version;
    std::string source;
    bool main = false;
    std::string build_system = "make";
    std::optional<std::string> custom_build;
    std::optional<std::string> custom_install;
};

struct ProjectConfig {
    PackageConfig package;
    BuildConfig build;
    std::vector<SourceConfig> sources;
};

ProjectConfig parse_config(const std::string& path);

} // namespace pmake::config
