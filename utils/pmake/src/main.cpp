#include <iostream>
#include <log.hpp>
#include <cli/arguments.hpp>
#include <config/parser.hpp>
#include <config/validation.hpp>
#include <build/manager.hpp>
#include <package/manifest.hpp>

int main(int argc, char* argv[])
{
    using namespace pmake::log;

    try {
        // Parse CLI arguments
        auto options = pmake::cli::parse_arguments(argc, argv);

        if (options.version) {
            std::cout << "PMAKE version 1.0.0 (C++20 Package Builder)\n";
            return 0;
        }

        if (options.help) {
            return 0;
        }

        if (!options.uninstall_path.empty()) {
            return pmake::package::uninstall_manifest(options.uninstall_path) ? 0 : 1;
        }

        // Reading configuration
        slogf(INFO, "Reading {}", options.config_path);
        auto config = pmake::config::parse_config(options.config_path);

        // Override jobs if specified via CLI
        if (options.jobs.has_value()) {
            config.build.jobs = options.jobs.value();
        }

        // Validating configuration
        slogf(INFO, "Validating configuration");
        pmake::config::validate_config(config, options.config_path);

        // Run build manager
        bool success = pmake::build::run_build_manager(config, options);
        if (!success) {
            return 1;
        }

        return 0;
    } catch (const std::exception& e) {
        slogf(FATAL, "{}", e.what());
        return 1;
    } catch (...) {
        slogf(FATAL, "Unknown fatal error occurred");
        return 1;
    }
}
