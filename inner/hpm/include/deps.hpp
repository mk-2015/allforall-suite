#pragma once

#include <string>

bool download_file(const std::string& url, const std::string& output_path);
bool extract_zip(const std::string& archive_path, const std::string& target_dir);