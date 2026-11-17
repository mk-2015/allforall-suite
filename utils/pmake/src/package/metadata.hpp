#pragma once

#include <config/parser.hpp>
#include <string>

namespace pmake::package {

std::string generate_metadata(const config::ProjectConfig& config);

} // namespace pmake::package
