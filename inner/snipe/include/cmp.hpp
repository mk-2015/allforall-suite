#pragma once

#include<filesystem>
#include<fstream>
#include<cstdlib>
#include<iostream>

namespace fs = std::filesystem;

fs::path GetSnipeInstallDir();
int CreateFileWithContents(fs::path file, std::string contents);
int CreateFileWithAppend(fs::path file, std::string contents);
