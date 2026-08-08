#include <pkg.hpp>
#include <pkg_db.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

fs::path get_home_dir() {
    const char* home = std::getenv("HOME");
    if (!home) {
        home = std::getenv("USERPROFILE"); // Fallback for Windows
    }
    if (!home) {
        return "";
    }
    return fs::path(home);
}

int hpm_init() {
    fs::path home = get_home_dir();
    if (home.empty()) {
        hpm_fail((int)NULL, "Could not find system HOME directory.\n");
        return 1;
    }

    fs::path hpm_dir = home / ".hpm";
    fs::path config_file = hpm_dir / ".config.yml";
    fs::path packages_dir = hpm_dir / "packages";
    fs::path db_file = hpm_dir / "db.sqlite";

    try {
        fs::create_directories(packages_dir);

        if (!fs::exists(config_file)) {
            std::ofstream cfg(config_file);
            if (cfg.is_open()) {
                cfg << "# hpm configuration\n";
                cfg << "version: 1.0.0\n";
                cfg << "repos:\n";
                cfg << "  - https://repo.example.com/hpm\n";
                cfg.close();
            }
        }

        if (!fs::exists(db_file)) {
            db_init();
        }

        hpm_ok("Successfully initialized HPM environment at %s\n", hpm_dir.c_str());
        return 0;

    } catch (const std::exception& e) {
        hpm_fail((int)NULL, "Failed to initialize HPM: %s\n", e.what());
        return 2;
    }
}