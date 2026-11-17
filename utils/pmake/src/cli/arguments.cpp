#include <cli/arguments.hpp>
#include <CLI/CLI.hpp>
#include <iostream>

namespace pmake::cli {

CliOptions parse_arguments(int argc, char* argv[])
{
    CLI::App app{"PMAKE - A C++20 Package Building Utility"};

    CliOptions options;

    app.add_option("-p,--config", options.config_path, "Path to configuration file (default: PMAKE.TML)");
    app.add_option("-j,--jobs", options.jobs, "Number of concurrent build jobs");
    app.add_flag("-s,--syncdeps", options.syncdeps, "Install missing dependencies");
    app.add_flag("-i,--install", options.install, "Install package after successful build");
    app.add_flag("-c,--clean", options.clean, "Clean build files and work directories");
    app.add_flag("-f,--force", options.force, "Force installation or overwrite existing packages");
    app.add_flag("-g,--geninteg", options.geninteg, "Generate source integrity checksums");
    app.add_flag("-S,--source", options.source_only, "Extract source files only, do not build");
    app.add_option("-u,--uninstall", options.uninstall_path, "Path to the package manifest file to uninstall");
    app.add_flag("-V,--version", options.version, "Display version information and exit");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        // Let CLI11 handle standard help/version/errors
        int exit_code = app.exit(e);
        if (exit_code == 0) {
            options.help = true;
        } else {
            std::exit(exit_code);
        }
    }

    if (app.get_help_ptr()->count() > 0) {
        options.help = true;
    }

    return options;
}

} // namespace pmake::cli
