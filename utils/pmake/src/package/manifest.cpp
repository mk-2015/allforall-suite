#include <package/manifest.hpp>
#include <toml++/toml.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace pmake::package {

namespace {

std::string escape_toml_string(const std::string& input)
{
    std::string out;
    out.reserve(input.size());
    for (char ch : input) {
        switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += ch; break;
        }
    }
    return out;
}

} // namespace

bool write_manifest(
    const std::string& manifest_path,
    const std::string& package_name,
    const std::vector<std::string>& installed_files
)
{
    std::ofstream out(manifest_path);
    if (!out) {
        return false;
    }

    out << "name = \"" << escape_toml_string(package_name) << "\"\n";
    out << "files = [\n";
    for (std::size_t i = 0; i < installed_files.size(); ++i) {
        out << "  \"" << escape_toml_string(installed_files[i]) << "\"";
        if (i + 1 < installed_files.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]\n";
    return true;
}

bool generate_manifest_for_directory(
    const std::filesystem::path& install_root,
    const std::string& manifest_path,
    const std::string& package_name
)
{
    if (!std::filesystem::exists(install_root)) {
        return false;
    }

    std::vector<std::string> files;
    std::filesystem::recursive_directory_iterator it(install_root), end;
    for (; it != end; ++it) {
        const auto& entry = *it;
        if (entry.is_regular_file()) {
            files.push_back(std::filesystem::absolute(entry.path()).string());
        }
    }

    return write_manifest(manifest_path, package_name, files);
}

bool uninstall_manifest(const std::string& manifest_path)
{
    std::filesystem::path path(manifest_path);
    if (!std::filesystem::exists(path)) {
        std::cerr << "Manifest not found: " << manifest_path << "\n";
        return false;
    }

    toml::table tbl;
    try {
        tbl = toml::parse_file(path.string());
    } catch (const toml::parse_error& e) {
        std::cerr << "Failed to parse manifest '" << manifest_path << "': " << e.description() << "\n";
        return false;
    }

    std::vector<std::string> files;
    if (auto files_node = tbl["files"].as_array()) {
        files_node->for_each([&](auto&& elem) {
            if (elem.is_string()) {
                files.push_back(elem.as_string()->get());
            }
        });
    }

    if (files.empty()) {
        std::cerr << "Manifest does not contain any install paths: " << manifest_path << "\n";
        return false;
    }

    for (const auto& file : files) {
        std::filesystem::path target(file);
        if (!std::filesystem::exists(target)) {
            std::cout << "Skipping missing file: " << file << "\n";
            continue;
        }

        if (std::filesystem::is_directory(target)) {
            std::filesystem::remove_all(target);
        } else {
            std::filesystem::remove(target);
        }

        std::cout << "Removed: " << file << "\n";
    }

    return true;
}

} // namespace pmake::package
