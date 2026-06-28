#include<cmp.hpp>

fs::path GetSnipeInstallDir() {
    #if defined(_WIN32)
        const char* home = std::getenv("USERPROFILE");
    #else
        const char* home = std::getenv("HOME");
    #endif

    if (home != nullptr) {
        return fs::path(home) / ".snipe";
    }

    return fs::path("/") / ".snipe";
}

int CreateFileWithContents(fs::path file, std::string contents)
{
    std::ofstream outputFile(file);
    if (!outputFile.is_open()) {
        std::cerr << "[SNIPE] Error opening " << file << "\n" << std::endl;
        return -1;
    }
    outputFile << contents;
    outputFile.close();
    return 0;
}

int CreateFileWithAppend(fs::path file, std::string contents) 
{
    std::ofstream outputFile(file, std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "[SNIPE] Error opening " << file << "\n" << std::endl;
        return -1;
    }
    outputFile << contents;
    outputFile.close();
    return 0;
}