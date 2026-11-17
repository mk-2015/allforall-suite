#pragma once

#include <config/parser.hpp>
#include <string>
#include <filesystem>

namespace pmake::build {

bool prepare_source(const config::SourceConfig& src_config, const config::BuildConfig& build_config);

} // namespace pmake::build
