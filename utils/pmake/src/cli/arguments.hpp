#pragma once

#include <string>
#include <optional>

namespace pmake::cli {

struct CliOptions {
    std::string config_path = "PMAKE.TML";
    std::optional<int> jobs;
    bool syncdeps = false;
    bool install = false;
    bool clean = false;
    bool force = false;
    bool geninteg = false;
    bool source_only = false;
    std::string uninstall_path;
    bool version = false;
    bool help = false;
};

CliOptions parse_arguments(int argc, char* argv[]);

} // namespace pmake::cli
