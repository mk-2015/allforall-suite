#pragma once

#include <string>
#include <vector>

namespace pmake::system {

int run_process(const std::string& command, const std::vector<std::string>& args, const std::string& working_dir = "");

} // namespace pmake::system
