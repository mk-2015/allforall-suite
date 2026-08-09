#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>

namespace fs = std::filesystem;

void print_lsdir(const std::string& path_str = ".") {
    fs::path target_path(path_str);
    
    if (!fs::exists(target_path)) {
        std::cerr << "lsdir: error: path does not exist: " << path_str << "\n";
        return;
    }

    if (!fs::is_directory(target_path)) {
        auto ftime = fs::last_write_time(target_path);
        std::cout << "[FILE] " << target_path.filename().string() << " (" << fs::file_size(target_path) << " bytes)\n";
        return;
    }

    std::cout << "Directory listing of: " << fs::absolute(target_path).string() << "\n";
    std::cout << std::string(50, '-') << "\n";

    for (const auto& entry : fs::directory_iterator(target_path)) {
        auto filename = entry.path().filename().string();
        
        if (entry.is_directory()) {
            std::cout << "  [DIR]  " << filename << "/\n";
        } else if (entry.is_regular_file()) {
            std::cout << "  [FILE] " << filename << " (" << entry.file_size() << " bytes)\n";
        } else {
            std::cout << "  [SPEC] " << filename << "\n";
        }
    }
    std::cout << std::string(50, '-') << "\n";
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        print_lsdir(argv[1]);
    } else {
        print_lsdir(".");
    }
    return 0;
}