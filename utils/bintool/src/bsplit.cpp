#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Options {
    std::string input_file;
    std::string output_dir;
    std::size_t chunk_size = 1024 * 1024;
    std::string prefix = "chunk";
    std::size_t pad_width = 1;
    bool show_help = false;
};

void print_help() {
    std::cout << "bsplit - Binary File Splitter\n\n"
              << "USAGE:\n"
              << "    bsplit [OPTIONS] <file>\n\n"
              << "OPTIONS:\n"
              << "    -s, --size <bytes>     Chunk size in bytes (default: 1048576)\n"
              << "    -o, --output <dir>     Output directory for split chunks\n"
              << "    -p, --prefix <name>    Chunk filename prefix (default: chunk)\n"
              << "    -z, --pad <digits>     Zero-pad chunk number to this width\n"
              << "    -h, --help             Display help menu\n";
}

bool parse_size(const std::string& value, std::size_t& out) {
    try {
        std::size_t parsed = std::stoull(value);
        if (parsed == 0) {
            return false;
        }
        out = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> positionals;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-s" || arg == "--size") {
            if (i + 1 >= argc || !parse_size(argv[++i], opts.chunk_size)) {
                return false;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 >= argc) {
                return false;
            }
            opts.output_dir = argv[++i];
        } else if (arg == "-p" || arg == "--prefix") {
            if (i + 1 >= argc) {
                return false;
            }
            opts.prefix = argv[++i];
        } else if (arg == "-z" || arg == "--pad") {
            if (i + 1 >= argc) {
                return false;
            }
            std::size_t digits = 0;
            if (!parse_size(argv[++i], digits)) {
                return false;
            }
            opts.pad_width = digits;
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

    opts.input_file = positionals[0];
    return true;
}

std::string format_index(std::size_t index, std::size_t width) {
    std::string text = std::to_string(index);
    if (width <= 1) {
        return text;
    }
    if (text.size() < width) {
        return std::string(width - text.size(), '0') + text;
    }
    return text;
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

    if (!fs::exists(opts.input_file)) {
        std::cerr << "Error: input file does not exist: " << opts.input_file << "\n";
        return 1;
    }

    fs::path input_path = opts.input_file;
    fs::path output_dir = opts.output_dir.empty()
        ? input_path.parent_path() / (input_path.filename().string() + ".split")
        : fs::path(opts.output_dir);

    if (fs::exists(output_dir) && !fs::is_directory(output_dir)) {
        std::cerr << "Error: output path is not a directory: " << output_dir << "\n";
        return 1;
    }

    fs::create_directories(output_dir);

    std::ifstream input(opts.input_file, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Error: unable to open input file for reading.\n";
        return 1;
    }

    std::vector<char> buffer(opts.chunk_size);
    std::size_t index = 1;
    bool wrote_any = false;

    while (true) {
        input.read(buffer.data(), static_cast<std::streamsize>(opts.chunk_size));
        std::streamsize bytes_read = input.gcount();
        if (bytes_read <= 0) {
            break;
        }

        std::string filename = opts.prefix + "-" + format_index(index, opts.pad_width) + ".bin";
        fs::path chunk_path = output_dir / filename;

        std::ofstream out(chunk_path, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            std::cerr << "Error: unable to create chunk file: " << chunk_path << "\n";
            return 1;
        }

        out.write(buffer.data(), bytes_read);
        out.close();

        std::cout << "Created " << chunk_path << "\n";
        ++index;
        wrote_any = true;
    }

    if (!wrote_any) {
        std::string filename = opts.prefix + "-" + format_index(1, opts.pad_width) + ".bin";
        fs::path chunk_path = output_dir / filename;
        std::ofstream out(chunk_path, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            std::cerr << "Error: unable to create empty chunk file: " << chunk_path << "\n";
            return 1;
        }
        out.close();
        std::cout << "Created empty file: " << chunk_path << "\n";
    }

    std::cout << "Split complete: " << output_dir << "\n";
    return 0;
}
