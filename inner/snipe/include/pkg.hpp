#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int DownloadPackage(const std::string& packageName, const std::string& packageVersion);
int InstallPackage(const std::string& packageName, const std::string& packageVersion);
int UntargzArchive(const fs::path& archivePath, const fs::path& destinationDir);
int RemovePackage(const std::string& packageName, const std::string& packageVersion);
int ExecTool(const std::string& packageName, const std::string& packageVersion, const std::string& toolName, const std::string& arguments);