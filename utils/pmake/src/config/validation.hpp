#pragma once

#include <config/parser.hpp>
#include <string>

namespace pmake::config {

void validate_config(const ProjectConfig& config, const std::string& filename = "PMAKE.TML");

} // namespace pmake::config
