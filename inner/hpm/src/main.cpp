#include <pkg.hpp>
#include <vector>
#include <pkg_db.hpp>

#include <iostream>
#include <string_view>

int main(int argc, char* argv[])
{
    std::cout << "hpm: Hypr Package Manager\n";

    if (argc < 2) {
        std::cout << "Usage: hpm [-i|-l|-r|-rf|-sr|-lr|init] [args...]\n";
        std::cout << "    -i    Install a package\n";
        std::cout << "    -l    List all installed packages\n";
        std::cout << "    -r    Remove a package\n";
        std::cout << "    -rf   Forcibly Remove a package\n";
        std::cout << "    -sr   Search a package from repositories\n";
        std::cout << "    -lr   List all repositories\n";
        std::cout << "    init  Initialize the package home\n";
        return 1;
    }

    std::vector<std::string> pkglist;

    for (int cnt = 1; cnt < argc; cnt++)
    {
        std::string_view argument = argv[cnt];

        if (argument == "-i") {
            while (cnt + 1 < argc && argv[cnt + 1][0] != '-') {
                pkglist.push_back(argv[++cnt]);
            }

            if (pkglist.empty()) {
                hpm_fail((int)NULL, "Error: '-i' requires at least one package name.\n");
                return 1;
            }

            hpm_install_packages(pkglist);
            break;
        }
        else if (argument == "-l") {
            std::vector<std::string> packages = db_list_installed_packages();
            
            if (packages.empty()) {
                printf("No packages currently installed.\n");
            } else {
                for (const auto& pkg_info : packages) {
                    printf("  - %s\n", pkg_info.c_str());
                }
                printf("Total installed: %zu\n", packages.size());
            }
        } 
        else if (argument == "-r") {
            while (cnt + 1 < argc && argv[cnt + 1][0] != '-') {
                pkglist.push_back(argv[++cnt]);
            }

            if (pkglist.empty()) {
                hpm_fail((int)NULL, "Error: '-r' requires at least one package name.\n");
                return 1;
            }

            hpm_uninstall_package(pkglist, false);
            break;
        } 
        else if (argument == "-rf") {
            while (cnt + 1 < argc && argv[cnt + 1][0] != '-') {
                pkglist.push_back(argv[++cnt]);
            }

            if (pkglist.empty()) {
                hpm_fail((int)NULL, "Error: '-rf' requires at least one package name.\n");
                return 1;
            }

            hpm_uninstall_package(pkglist, true);
            break;
        } 
        else if (argument == "-lr") {
            hpm_info("Listing configured repositories...\n");

            std::vector<std::string> repos = load_repo_urls();

            if (repos.empty()) {
                printf("No repositories configured in ~/.hpm/.config.yml\n");
            } else {
                for (size_t i = 0; i < repos.size(); ++i) {
                    printf("  [%zu] %s\n", i + 1, repos[i].c_str());
                }
                printf("Total repositories: %zu\n", repos.size());
            }
        }      
        else if (argument == "-sr" || argument == "--search-repo") {
            if (argc < 3) {
                hpm_fail((int)NULL, "Error: No search pattern provided.\nUsage: hpm -sr <regex_pattern>\n");
            } else {
                std::string pattern = argv[2];
                hpm_info("Searching remote repositories for regex '%s'...\n", pattern.c_str());

                std::vector<RemotePackageInfo> matches = search_remote_repositories(pattern);

                if (matches.empty()) {
                    printf("No matching packages found across remote repositories.\n");
                } else {
                    printf("Found %zu matching package(s):\n", matches.size());
                    for (const auto& match : matches) {
                        printf("  - %-25s [Repo: %s]\n", match.name.c_str(), match.repo_url.c_str());
                    }
                }
                break;
            }
        }
        else if (argument == "init") {
            return hpm_init();
        } 
        else {
            hpm_fail((int)NULL, "Error: invalid Argument %s\n", argv[cnt]);
        }
    } 

    return 0;
}