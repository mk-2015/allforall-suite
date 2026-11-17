#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace pmake::package {

bool write_manifest(
    const std::string& manifest_path,
    const std::string& package_name,
    const std::vector<std::string>& installed_files
);

bool generate_manifest_for_directory(
    const std::filesystem::path& install_root,
    const std::string& manifest_path,
    const std::string& package_name
);

bool uninstall_manifest(const std::string& manifest_path);

} // namespace pmake::package
