#include <system/filesystem.hpp>
#include <system/process.hpp>
#include <iostream>

namespace pmake::system {

bool create_directory(const std::filesystem::path& path)
{
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    return !ec;
}

bool remove_path(const std::filesystem::path& path)
{
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    return !ec;
}

bool exists(const std::filesystem::path& path)
{
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

bool extract_archive(const std::filesystem::path& archive_path, const std::filesystem::path& destination)
{
    if (!std::filesystem::exists(archive_path)) {
        return false;
    }

    std::filesystem::create_directories(destination);

    std::string ext = archive_path.extension().string();
    // Handle double extensions like .tar.gz or .tar.xz or .tar.zst
    std::string filename = archive_path.filename().string();

    std::string cmd = "tar";
    std::vector<std::string> args;

    if (filename.find(".tar.gz") != std::string::npos || filename.find(".tgz") != std::string::npos) {
        args = {"-xzf", archive_path.string(), "-C", destination.string()};
    } else if (filename.find(".tar.xz") != std::string::npos || filename.find(".txz") != std::string::npos) {
        args = {"-xJf", archive_path.string(), "-C", destination.string()};
    } else if (filename.find(".tar.zst") != std::string::npos) {
        args = {"--use-compress-program=unzstd", "-xf", archive_path.string(), "-C", destination.string()};
    } else if (filename.find(".tar.bz2") != std::string::npos || filename.find(".tbz2") != std::string::npos) {
        args = {"-xjf", archive_path.string(), "-C", destination.string()};
    } else if (filename.find(".tar") != std::string::npos) {
        args = {"-xf", archive_path.string(), "-C", destination.string()};
    } else if (filename.find(".zip") != std::string::npos) {
        cmd = "unzip";
        args = {"-q", archive_path.string(), "-d", destination.string()};
    } else {
        // Default fallback to tar
        args = {"-xf", archive_path.string(), "-C", destination.string()};
    }

    int exit_code = run_process(cmd, args, "");
    return exit_code == 0;
}

} // namespace pmake::system
