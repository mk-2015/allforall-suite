#pragma once

#include <string>
#include <filesystem>

namespace pmake::system {

bool create_directory(const std::filesystem::path& path);
bool remove_path(const std::filesystem::path& path);
bool exists(const std::filesystem::path& path);
bool extract_archive(const std::filesystem::path& archive_path, const std::filesystem::path& destination);

} // namespace pmake::system
