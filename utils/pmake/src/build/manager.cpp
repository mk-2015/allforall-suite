#include <build/manager.hpp>
#include <build/source.hpp>
#include <build/executor.hpp>
#include <log.hpp>
#include <package/manifest.hpp>
#include <iostream>

namespace pmake::build {

bool run_build_manager(const config::ProjectConfig& config, const cli::CliOptions& options)
{
    // Find main and secondary sources
    std::vector<const config::SourceConfig*> secondary_sources;
    const config::SourceConfig* main_source = nullptr;

    for (const auto& src : config.sources) {
        if (src.main) {
            if (main_source != nullptr) {
                pmake::log::slogf(pmake::log::ERROR, "Multiple main sources found!");
                return false;
            }
            main_source = &src;
        } else {
            secondary_sources.push_back(&src);
        }
    }

    if (!main_source) {
        pmake::log::slogf(pmake::log::ERROR, "No main source found with main = true");
        return false;
    }

    // Step 1: Prepare all sources (download & extract)
    pmake::log::slogf(pmake::log::INFO, "Preparing sources");
    for (const auto& src : config.sources) {
        if (!prepare_source(src, config.build)) {
            return false;
        }
    }

    if (options.source_only) {
        pmake::log::slogf(pmake::log::INFO, "Source-only extraction complete (-S flag specified)");
        return true;
    }

    // Phase 1 — Build secondary packages
    if (!secondary_sources.empty()) {
        pmake::log::slogf(pmake::log::INFO, "Building secondary packages");
        for (const auto* src : secondary_sources) {
            if (!build_source(*src, config.build)) {
                pmake::log::slogf(pmake::log::ERROR, "Secondary package '{}' build failed. Stopping immediately.", src->name);
                return false;
            }
        }

        // Phase 2 — Install secondary packages
        pmake::log::slogf(pmake::log::INFO, "Installing secondary packages");
        for (const auto* src : secondary_sources) {
            if (!install_source(*src, config.build)) {
                pmake::log::slogf(pmake::log::ERROR, "Secondary package '{}' installation failed. Stopping immediately.", src->name);
                return false;
            }
        }
    }

    // Phase 3 — Build main package
    pmake::log::slogf(pmake::log::INFO, "Building main package");
    if (!build_source(*main_source, config.build)) {
        pmake::log::slogf(pmake::log::ERROR, "Main package '{}' build failed.", main_source->name);
        return false;
    }

    // Phase 4 — Install main package (if requested)
    if (options.install) {
        pmake::log::slogf(pmake::log::INFO, "Installing main package");
        if (!install_source(*main_source, config.build)) {
            pmake::log::slogf(pmake::log::ERROR, "Main package '{}' installation failed.", main_source->name);
            return false;
        }

        std::filesystem::path install_root = std::filesystem::absolute(std::filesystem::path(config.build.output));
        std::string manifest_path = config.package.name + "-manifest.toml";
        if (!pmake::package::generate_manifest_for_directory(install_root, manifest_path, config.package.name)) {
            pmake::log::slogf(pmake::log::WARNING, "Failed to generate install manifest '{}' for package '{}'", manifest_path, config.package.name);
        } else {
            pmake::log::slogf(pmake::log::INFO, "Package manifest written to {}", manifest_path);
        }
    } else {
        pmake::log::slogf(pmake::log::INFO, "Main package built successfully (use -i or --install to install)");
    }

    return true;
}

} // namespace pmake::build
