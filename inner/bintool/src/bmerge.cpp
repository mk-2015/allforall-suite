#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

struct Options {
    std::string input_dir;
    std::string output_file;
    std::string prefix = "chunk";
    bool show_help = false;
};

void print_help() {
    std::cout << "bmerge - Binary File Merger\n\n"
              << "USAGE:\n"
              << "    bmerge [OPTIONS] <split-dir>\n\n"
              << "OPTIONS:\n"
              << "    -o, --output <file>    Output file path for the restored binary\n"
              << "    -p, --prefix <name>    Chunk filename prefix (default: chunk)\n"
              << "    -h, --help             Display this help menu\n";
}

bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> positionals;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                return false;
            }
            opts.output_file = argv[++i];
        } else if (arg == "-p" || arg == "--prefix") {
            if (i + 1 >= argc) {
                return false;
            }
            opts.prefix = argv[++i];
        } else if (arg[0] != '-') {
            positionals.push_back(arg);
        } else {
            return false;
        }
    }

    if (opts.show_help) {
        return true;
    }

    if (positionals.size() != 1) {
        return false;
    }

    opts.input_dir = positionals[0];
    return true;
}

bool extract_chunk_index(const std::string& filename, const std::string& prefix, std::size_t& value) {
    std::string name = filename;
    if (name.rfind(prefix, 0) != 0) {
        return false;
    }

    std::string suffix = name.substr(prefix.size());
    if (suffix.empty() || suffix[0] != '-') {
        return false;
    }

    suffix = suffix.substr(1);
    std::string number_part;
    for (char ch : suffix) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            number_part += ch;
        } else if (!number_part.empty()) {
            break;
        } else {
            return false;
        }
    }

    if (number_part.empty()) {
        return false;
    }

    std::string ext = fs::path(filename).extension().string();
    if (ext != ".bin" && ext != ".dat" && ext != ".chunk" && ext != "") {
        return false;
    }

    try {
        value = static_cast<std::size_t>(std::stoull(number_part));
        return true;
    } catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {
    Options opts;

    if (!parse_args(argc, argv, opts)) {
        print_help();
        return 1;
    }

    if (opts.show_help) {
        print_help();
        return 0;
    }

    if (!fs::exists(opts.input_dir) || !fs::is_directory(opts.input_dir)) {
        std::cerr << "Error: input directory does not exist: " << opts.input_dir << "\n";
        return 1;
    }

    std::vector<std::pair<std::size_t, fs::path>> chunks;
    for (const auto& entry : fs::directory_iterator(opts.input_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        std::size_t index = 0;
        if (extract_chunk_index(entry.path().filename().string(), opts.prefix, index)) {
            chunks.push_back({index, entry.path()});
        }
    }

    if (chunks.empty()) {
        std::cerr << "Error: no chunk files matching prefix '" << opts.prefix << "-*' were found.\n";
        return 1;
    }

    std::sort(chunks.begin(), chunks.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    fs::path output_path = opts.output_file.empty()
        ? fs::path(opts.input_dir).parent_path() / "merged.bin"
        : fs::path(opts.output_file);

    std::ofstream out(output_path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Error: unable to create output file: " << output_path << "\n";
        return 1;
    }

    std::size_t total_written = 0;
    for (const auto& [index, chunk_path] : chunks) {
        std::ifstream in(chunk_path, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Error: unable to open chunk file: " << chunk_path << "\n";
            return 1;
        }

        char buffer[65536];
        while (in.read(buffer, sizeof(buffer)) || in.gcount() > 0) {
            out.write(buffer, in.gcount());
            total_written += static_cast<std::size_t>(in.gcount());
        }
    }

    out.close();
    std::cout << "Merged " << chunks.size() << " chunks into " << output_path << " (" << total_written << " bytes)\n";
    return 0;
}
