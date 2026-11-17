#include <build/executor.hpp>
#include <system/process.hpp>
#include <system/filesystem.hpp>
#include <log.hpp>
#include <iostream>

namespace pmake::build {

// Helper to find the actual source directory (in case archive extracted into a subdirectory)
std::filesystem::path get_source_root(const std::filesystem::path& work_dir)
{
    // If there is exactly one top-level extracted directory, use it as the project root.
    // Do not discard directories just because they are named "pkg" or "build";
    // those names are valid source roots and may appear in real archives.
    int sub_count = 0;
    std::filesystem::path sub_dir = work_dir;
    for (const auto& entry : std::filesystem::directory_iterator(work_dir)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (name == ".deps") {
                continue;
            }
            sub_dir = entry.path();
            sub_count++;
        }
    }
    return (sub_count == 1) ? sub_dir : work_dir;
}

bool build_source(const config::SourceConfig& src_config, const config::BuildConfig& build_config)
{
    std::filesystem::path work_dir = std::filesystem::path(build_config.directory) / src_config.id;
    std::filesystem::path src_root = get_source_root(work_dir);

    std::string bs = src_config.build_system;
    std::string jobs_str = std::to_string(build_config.jobs);

    pmake::log::slogf_tabbed(pmake::log::INFO, "Building '{}' using {} (jobs: {})", src_config.name, bs, build_config.jobs);

    int exit_code = 0;

    if (bs == "make") {
        exit_code = system::run_process("make", {"-j" + jobs_str}, src_root.string());
    } else if (bs == "cmake") {
        std::filesystem::path build_subdir = src_root / "build";
        system::create_directory(build_subdir);
        exit_code = system::run_process("cmake", {"-B", build_subdir.string(), "-S", src_root.string(), "-DCMAKE_BUILD_TYPE=Release"}, src_root.string());
        if (exit_code == 0) {
            exit_code = system::run_process("cmake", {"--build", build_subdir.string(), "-j", jobs_str}, src_root.string());
        }
    } else if (bs == "meson") {
        std::filesystem::path build_subdir = src_root / "build";
        exit_code = system::run_process("meson", {"setup", build_subdir.string(), src_root.string()}, src_root.string());
        if (exit_code == 0) {
            exit_code = system::run_process("meson", {"compile", "-C", build_subdir.string()}, src_root.string());
        }
    } else if (bs == "autotools") {
        // Check if configure exists, else run autoreconf -ivf
        if (!system::exists(src_root / "configure")) {
            system::run_process("autoreconf", {"-ivf"}, src_root.string());
        }
        exit_code = system::run_process("./configure", {}, src_root.string());
        if (exit_code == 0) {
            exit_code = system::run_process("make", {"-j" + jobs_str}, src_root.string());
        }
    } else if (bs == "custom") {
        if (src_config.custom_build) {
            exit_code = system::run_process("sh", {"-c", *src_config.custom_build}, src_root.string());
        } else {
            pmake::log::slogf(pmake::log::ERROR, "Custom build specified for '{}' but no custom_build command provided", src_config.name);
            return false;
        }
    } else {
        pmake::log::slogf(pmake::log::ERROR, "Unsupported build system '{}' for '{}'", bs, src_config.name);
        return false;
    }

    if (exit_code != 0) {
        pmake::log::slogf(pmake::log::ERROR, "Build failed for '{}'", src_config.name);
        return false;
    }

    return true;
}

bool install_source(const config::SourceConfig& src_config, const config::BuildConfig& build_config)
{
    std::filesystem::path work_dir = std::filesystem::path(build_config.directory) / src_config.id;
    std::filesystem::path src_root = get_source_root(work_dir);
    std::filesystem::path install_root = std::filesystem::absolute(std::filesystem::path(build_config.output));

    std::string bs = src_config.build_system;
    pmake::log::slogf_tabbed(pmake::log::INFO, "Installing '{}' using {} (staging to {})", src_config.name, bs, install_root.string());

    int exit_code = 0;

    if (bs == "make") {
        exit_code = system::run_process("make", {"install", "DESTDIR=" + install_root.string()}, src_root.string());
    } else if (bs == "cmake") {
        std::filesystem::path build_subdir = src_root / "build";
        exit_code = system::run_process("cmake", {"--install", build_subdir.string(), "--prefix", install_root.string()}, src_root.string());
    } else if (bs == "meson") {
        std::filesystem::path build_subdir = src_root / "build";
        exit_code = system::run_process("meson", {"install", "-C", build_subdir.string(), "--destdir", install_root.string()}, src_root.string());
    } else if (bs == "autotools") {
        exit_code = system::run_process("make", {"install", "DESTDIR=" + install_root.string()}, src_root.string());
    } else if (bs == "custom") {
        if (src_config.custom_install) {
            exit_code = system::run_process("sh", {"-c", *src_config.custom_install}, src_root.string());
        } else {
            // Optional custom install - if not provided, might be a no-op or success
            pmake::log::slogf_tabbed(pmake::log::INFO, "No custom install command specified for '{}', skipping installation step", src_config.name);
            return true;
        }
    } else {
        pmake::log::slogf(pmake::log::ERROR, "Unsupported installation build system '{}' for '{}'", bs, src_config.name);
        return false;
    }

    if (exit_code != 0) {
        pmake::log::slogf(pmake::log::ERROR, "Installation failed for '{}'", src_config.name);
        return false;
    }

    return true;
}

} // namespace pmake::build
