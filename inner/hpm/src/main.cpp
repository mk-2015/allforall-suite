#include <pkg.hpp>
#include <vector>
#include <pkg_db.hpp>

#include <iostream>
#include <string_view>
#include <filesystem>

int main(int argc, char* argv[])
{
    std::cout << "hpm: Hypr Package Manager\n";

    if (argc < 2) {
        std::cout << "Usage: hpm [-i|--install|-l|--list|-r|--remove|-rf|--force-remove|-sr|--search-repo|-lr|--list-repos|init|key] [args...]\n";
        std::cout << "    -i, --install           Install a package\n";
        std::cout << "    -l, --list              List all installed packages\n";
        std::cout << "    -r, --remove            Remove a package\n";
        std::cout << "    -rf, --force-remove     Forcibly Remove a package\n";
        std::cout << "    -sr, --search-repo      Search a package from repositories\n";
        std::cout << "    -lr, --list-repos       List all repositories\n";
        std::cout << "    -u , --upgrade          Upgrade alll installed packages\n";
        std::cout << "    init                    Initialize the package home\n";
        std::cout << "    key add                 Add a maintainer's public key\n";
        std::cout << "    key remove              Remove a maintainer's public key\n";
        std::cout << "    key list                List trusted maintainers\n";
        std::cout << "    key generate            Generate a new Ed25519 keypair\n";
        std::cout << "    key extract             Extract the public Ed25519\n";
        std::cout << "    key sign                Sign an archive with the maintainer's private key\n";
        return 1;
    }

    std::vector<std::string> pkglist;

    for (int cnt = 1; cnt < argc; cnt++)
    {
        std::string_view argument = argv[cnt];

        if (argument == "-i" || argument == "--install") {
            while (cnt + 1 < argc && argv[cnt + 1][0] != '-') {
                pkglist.push_back(argv[++cnt]);
            }

            if (pkglist.empty()) {
                hpm_fail((int)NULL, "Error: '%s' requires at least one package name.\n", std::string(argument).c_str());
                return 1;
            }

            hpm_install_packages(pkglist);
            break;
        }
        else if (argument == "-l" || argument == "--list") {
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
        else if (argument == "-r" || argument == "--remove") {
            while (cnt + 1 < argc && argv[cnt + 1][0] != '-') {
                pkglist.push_back(argv[++cnt]);
            }

            if (pkglist.empty()) {
                hpm_fail((int)NULL, "Error: '%s' requires at least one package name.\n", std::string(argument).c_str());
                return 1;
            }

            hpm_uninstall_package(pkglist, false);
            break;
        } 
        else if (argument == "-rf" || argument == "--force-remove") {
            while (cnt + 1 < argc && argv[cnt + 1][0] != '-') {
                pkglist.push_back(argv[++cnt]);
            }

            if (pkglist.empty()) {
                hpm_fail((int)NULL, "Error: '%s' requires at least one package name.\n", std::string(argument).c_str());
                return 1;
            }

            hpm_uninstall_package(pkglist, true);
            break;
        } 
        else if (argument == "-lr" || argument == "--list-repos") {
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
        } else if (argument == "-u" || argument == "--upgrade") {
            hpm_upgrade_packages();
            break;
        } else if (argument == "key") {
            if (cnt + 1 >= argc) {
                hpm_fail(1, "Error: 'key' subcommand requires an action (add, remove, list, generate).\n");
            }
            std::string subaction = argv[++cnt];

            if (subaction == "add") {
                if (cnt + 2 >= argc) {
                    hpm_fail(1, "Usage: hpm key add <maintainer_name> <path_to_pubkey>\n");
                }
                std::string name = argv[++cnt];
                std::filesystem::path keyfile = argv[++cnt];
                keyring_add_maintainer(name, keyfile);
            }
            else if (subaction == "remove") {
                if (cnt + 1 >= argc) {
                    hpm_fail(1, "Usage: hpm key remove <maintainer_name>\n");
                }
                std::string name = argv[++cnt];
                keyring_remove_maintainer(name);

            } else if (subaction == "list") {
                std::vector<std::string> maintainers = keyring_list_maintainers();
                if (maintainers.empty()) {
                    printf("No maintainer public keys found in keyring.\n");
                } else {
                    printf("Trusted maintainers in keyring (%zu):\n", maintainers.size());
                    for (const auto& m : maintainers) {
                        printf("  - %s\n", m.c_str());
                    }
                }
            } else if (subaction == "generate") {
                if (cnt + 3 >= argc) {
                    hpm_fail(1, "Usage: hpm key generate <name> <email> <private_key_output_path>\n");
                }
                std::string name = argv[++cnt];
                std::string email = argv[++cnt];
                std::filesystem::path priv_path = argv[++cnt];
                keyring_generate_maintainer(name, email, priv_path);
            } else if (subaction == "extract") {
                if (cnt + 2 >= argc) {
                    hpm_fail(1, "Usage: hpm key extract <maintainer_name> <output_path>\n");
                }
                std::string name = argv[++cnt];
                std::filesystem::path out_path = argv[++cnt];
                keyring_extract_maintainer(name, out_path);
            } else if (subaction == "sign") {
                if (cnt + 3 >= argc) {
                    hpm_fail(1, "Usage: hpm key sign <private_key_path> <file_to_sign> <output_sig_path>\n");
                }
                std::filesystem::path priv_key = argv[++cnt];
                std::filesystem::path target_file = argv[++cnt];
                std::filesystem::path out_sig = argv[++cnt];
                keyring_sign_archive(priv_key, target_file, out_sig);
            } else {
                hpm_fail(1, "Error: Unknown key subcommand '%s'\n", subaction.c_str());
            }
            break;
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