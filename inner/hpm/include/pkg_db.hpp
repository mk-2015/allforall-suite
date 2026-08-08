#pragma once

#include <string>
#include <vector>

bool db_init();
bool db_is_package_installed(const std::string& pkg_name);
bool db_register_package(const std::string& pkg_name, const std::string& version, const std::vector<std::string>& installed_files);
bool db_remove_package(const std::string& pkg_name);
std::vector<std::string> db_get_dependents(const std::string& pkg_name);
std::vector<std::string> db_list_installed_packages();


bool verify_with_maintainer_key(const std::string& maintainer_name, 
                                const std::vector<unsigned char>& file_data, 
                                const std::vector<unsigned char>& signature);