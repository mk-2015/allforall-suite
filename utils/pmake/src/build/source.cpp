#include <build/source.hpp>
#include <system/filesystem.hpp>
#include <system/process.hpp>
#include <log.hpp>
#include <iostream>

namespace pmake::build {

bool prepare_source(const config::SourceConfig& src_config, const config::BuildConfig& build_config)
{
    std::filesystem::path work_dir = std::filesystem::path(build_config.directory) / src_config.id;
    system::create_directory(work_dir);

    std::string source_url = src_config.source;
    bool is_url = (source_url.rfind("http://", 0) == 0 ||
                   source_url.rfind("https://", 0) == 0 ||
                   source_url.rfind("ftp://", 0) == 0 ||
                   source_url.rfind("git://", 0) == 0 ||
                   source_url.rfind("ssh://", 0) == 0 ||
                   source_url.rfind("git@", 0) == 0);

    bool is_git_source = false;
    if (source_url.size() >= 4 && source_url.substr(source_url.size() - 4) == ".git") {
        is_git_source = true;
    }

    std::filesystem::path source_path = source_url;
    if (!is_url && std::filesystem::exists(source_path)) {
        std::filesystem::path git_dir = source_path / ".git";
        is_git_source = is_git_source || std::filesystem::exists(git_dir);
    }

    if (is_git_source) {
        std::filesystem::path clone_dir = work_dir / "repo";

        pmake::log::slogf_tabbed(pmake::log::INFO, "Cloning Git source for '{}' from {}", src_config.name, source_url);
        int exit_code = system::run_process("git", {"clone", "--depth", "1", source_url, clone_dir.string()}, work_dir.string());
        if (exit_code != 0) {
            pmake::log::slogf(pmake::log::ERROR, "Failed to clone Git source for '{}' from {}", src_config.name, source_url);
            return false;
        }

        return true;
    }

    std::filesystem::path archive_path;

    if (is_url) {
        auto last_slash = source_url.find_last_of('/');
        std::string filename = (last_slash != std::string::npos) ? source_url.substr(last_slash + 1) : "source.tar.gz";
        archive_path = work_dir / filename;

        pmake::log::slogf_tabbed(pmake::log::INFO, "Downloading source for '{}' from {}", src_config.name, source_url);

        int exit_code = system::run_process("curl", {"-L", "-s", "-S", "-o", archive_path.string(), source_url});
        if (exit_code != 0) {
            pmake::log::slogf(pmake::log::ERROR, "Failed to download source from {}", source_url);
            return false;
        }
    } else {
        archive_path = source_url;
        if (!system::exists(archive_path)) {
            pmake::log::slogf(pmake::log::ERROR, "Local source path does not exist: {}", source_url);
            return false;
        }
    }

    pmake::log::slogf_tabbed(pmake::log::INFO, "Extracting source for '{}'", src_config.name);
    bool extracted = system::extract_archive(archive_path, work_dir);
    if (!extracted) {
        pmake::log::slogf(pmake::log::ERROR, "Failed to extract archive for '{}'", src_config.name);
        return false;
    }

    return true;
}

} // namespace pmake::build
