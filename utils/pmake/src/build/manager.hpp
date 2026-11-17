#pragma once

#include <config/parser.hpp>
#include <cli/arguments.hpp>

namespace pmake::build {

bool run_build_manager(const config::ProjectConfig& config, const cli::CliOptions& options);

} // namespace pmake::build
