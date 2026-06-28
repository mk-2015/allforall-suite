#include<print>
#include<filesystem>
#include<string>
#include<iostream>
#include<cmp.hpp>

namespace fs = std::filesystem;

#define SNIPE_VERSION "0.0.1"

int RunInitlization() 
{
    fs::path installDir = GetSnipeInstallDir();
    char choice;

    std::println("[SNIPE] Initlaizing installation");
    if (fs::create_directory(installDir)) 
    {
        std::println("[SNIPE] Success to create install directory.");
    } else {
        std::println("[SNIPE] Failure to create directory.");
        std::println("[NOTE] Maybe directory exists?");
 
        while (true) 
        {
            std::print("Do you want to continue? (Y/N): ");
            std::cin >> choice;

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (choice == 'Y' || choice == 'y') {
                std::println("[SNIPE] Continuing...");
                break;
            } else if (choice == 'N' || choice == 'n') {
                return -1;
            } else {
                std::print("Invalid input. Please enter 'Y' or 'N'.\n\n");
            }
        }
    }

    int returnval = CreateFileWithContents(installDir / "version", std::string(SNIPE_VERSION));
    if (returnval < 0)
    {
        return -2;
    }

    while (true) 
    {
        std::print("Do you want to continue replacing package lists? (Y/N): ");
        std::cin >> choice;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 'Y' || choice == 'y') {
            std::string contents = "# packrepo.lst\n"
                                   "# example repo: https://repo1.archrepo.com/\n"
                                   "# format: https://repo1.archrepo.com/packagename/version/contents\n"
                                   "# latest: https://repo1.archrepo.com/packagename/.latest\n\n";
            returnval = CreateFileWithAppend(installDir / "packrepo.lst", contents);
            if (returnval < 0)
            {
                #ifdef _STRICT_MODE
                std::println("[SNIPE] Failure to create or replace package list file.");
                return -2;
                #else
                std::println("[SNIPE] Failure to create or replace package list file. Continuing...");
                #endif
            }

            break;
        } else if (choice == 'N' || choice == 'n') {
            break;
        } else {
            std::print("Invalid input. Please enter 'Y' or 'N'.\n\n");
        }
    }

    if (fs::create_directory(installDir / "packages")) 
    {
        std::println("[SNIPE] Success to create package directory.");
    } else 
    {
        std::println("[SNIPE] Failure to create package directory.");
        while (true) 
        {
            std::print("Do you want to continue? (Y/N): ");
            std::cin >> choice;

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (choice == 'Y' || choice == 'y') {
                std::println("[SNIPE] Continuing...");
                break;
            } else if (choice == 'N' || choice == 'n') {
                return -1;
            } else {
                std::print("Invalid input. Please enter 'Y' or 'N'.\n\n");
            }
        }
    }

    if (!fs::create_directory(installDir / "cache") && !fs::exists(installDir / "cache")) 
    {
        std::println("[SNIPE] Failure to create cache directory.");
        return -3; 
    }

    std::println("[SNIPE] Successfully initialized sniper.");
    return 0;
}

int ListPackages()
{
    fs::path installDir = GetSnipeInstallDir();
    fs::path pkgDir = installDir / "packages";

    if (!fs::exists(pkgDir) || !fs::is_directory(pkgDir))
    {
        std::println("[SNIPE] Error: Package directory does not exist. Run 'init' first.");
        return -1;
    }

    bool foundPackages = false;
    int totalPackages = 0;

    for (const auto& entry : fs::directory_iterator(pkgDir))
    {
        if (entry.is_directory())
        {
            std::println("  - {}", entry.path().filename().string());
            foundPackages = true;
            totalPackages++;
        }
    }

    std::println("Total packages: {}", totalPackages);

    if (!foundPackages)
    {
        std::println("[SNIPE] No packages currently installed.");
    }
    else 
    {
        std::println("Total packages: {}", totalPackages);
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::println(std::cerr, "Usage: snipe [command]");
        std::println(std::cerr, "Available commands: init");
        return 1;
    }

    std::string_view command = argv[1];

    if (command == "init") 
    {
        return RunInitlization();
    } else if (command == "list") 
    {
        return ListPackages();
    }
    
    std::println(std::cerr, "Error: Unknown command '{}'", command);
    return 1;
}