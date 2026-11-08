#include <iostream>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <cstdlib>

namespace fs = std::filesystem;

struct Options {
    std::string directory;
    std::string command;
    bool recursive = false;          // -s
    bool files_only = false;         // -f
    bool dirs_only = false;          // -d
    bool ignore_hidden = true;       // -n (default)
};

bool is_hidden(const fs::path& path) {
    auto filename = path.filename().string();
    return !filename.empty() && filename[0] == '.';
}

void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [directory] [command] [OPTIONS]\n"
              << "Options:\n"
              << "    -s    Watch subfolders too\n"
              << "    -f    Watch files only\n"
              << "    -d    Watch directories only\n"
              << "    -n    Don't watch hidden files and folders (default)\n"
              << "    -N    Watch hidden files and folders as well\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    Options opts;
    opts.directory = argv[1];
    opts.command = argv[2];

    // Parse options
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s") opts.recursive = true;
        else if (arg == "-f") opts.files_only = true;
        else if (arg == "-d") opts.dirs_only = true;
        else if (arg == "-n") opts.ignore_hidden = true;
        else if (arg == "-N") opts.ignore_hidden = false;
    }

    if (!fs::exists(opts.directory) || !fs::is_directory(opts.directory)) {
        std::cerr << "Error: Target directory does not exist or is not a directory.\n";
        return 1;
    }

    std::cout << "Watching directory: " << opts.directory << "\n"
              << "Command: " << opts.command << "\n"
              << "Recursive: " << (opts.recursive ? "Yes" : "No") << " | "
              << "Files only: " << (opts.files_only ? "Yes" : "No") << " | "
              << "Dirs only: " << (opts.dirs_only ? "Yes" : "No") << " | "
              << "Ignore hidden: " << (opts.ignore_hidden ? "Yes" : "No") << "\n\n";

    // Map to store path -> last write time
    std::unordered_map<std::string, fs::file_time_type> cache;

    auto scan_snapshot = [&]() {
        std::unordered_map<std::string, fs::file_time_type> current_snapshot;
        
        auto process_entry = [&](const fs::path& p) {
            if (opts.ignore_hidden && is_hidden(p)) return;

            bool is_dir = fs::is_directory(p);
            bool is_file = fs::is_regular_file(p);

            if (opts.files_only && !is_file) return;
            if (opts.dirs_only && !is_dir) return;
            if (!is_file && !is_dir) return; // Skip sockets, symlinks, etc. if needed

            try {
                auto lwt = fs::last_write_time(p);
                current_snapshot[p.string()] = lwt;
            } catch (...) {
                // Handle permission or race condition errors gracefully
            }
        };

        if (opts.recursive) {
            for (auto const& entry : fs::recursive_directory_iterator(opts.directory)) {
                process_entry(entry.path());
            }
        } else {
            for (auto const& entry : fs::directory_iterator(opts.directory)) {
                process_entry(entry.path());
            }
        }

        return current_snapshot;
    };

    // Note: fixing recursive directory iterator syntax namespace standard
    // Using standard fs::recursive_directory_iterator below in actual collection
    auto collect_snapshot = [&]() {
        std::unordered_map<std::string, fs::file_time_type> current_snapshot;
        
        auto process_entry = [&](const fs::path& p) {
            if (opts.ignore_hidden && is_hidden(p)) return;

            bool is_dir = fs::is_directory(p);
            bool is_file = fs::is_regular_file(p);

            if (opts.files_only && !is_file) return;
            if (opts.dirs_only && !is_dir) return;
            if (!is_file && !is_dir) return;

            try {
                current_snapshot[p.string()] = fs::last_write_time(p);
            } catch (...) {}
        };

        if (opts.recursive) {
            for (auto const& entry : fs::recursive_directory_iterator(opts.directory)) {
                process_entry(entry.path());
            }
        } else {
            for (auto const& entry : fs::directory_iterator(opts.directory)) {
                process_entry(entry.path());
            }
        }
        return current_snapshot;
    };

    // Initial baseline scan
    cache = collect_snapshot();

    // Main polling loop (checks every 500ms)
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto new_snapshot = collect_snapshot();
        bool changed = false;

        // Check for modifications or new files/directories
        for (const auto& [path, time] : new_snapshot) {
            auto it = cache.find(path);
            if (it == cache.end()) {
                std::cout << "[Created] " << path << "\n";
                changed = true;
            } else if (it->second != time) {
                std::cout << "[Modified] " << path << "\n";
                changed = true;
            }
        }

        // Check for deletions
        for (const auto& [path, time] : cache) {
            if (new_snapshot.find(path) == new_snapshot.end()) {
                std::cout << "[Deleted] " << path << "\n";
                changed = true;
            }
        }

        if (changed) {
            std::cout << "Executing command: " << opts.command << "\n";
            int result = std::system(opts.command.c_str());
            if (result != 0) {
                std::cerr << "Command exited with code " << result << "\n";
            }
            // Update cache after execution to avoid rapid re-triggering loops
            cache = std::move(new_snapshot);
        }
    }

    return 0;
}