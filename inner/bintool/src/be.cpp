#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

struct Options {
    std::string target_file;
    bool show_percentage = false;
    bool block_mode = false;
    std::size_t block_size = 1024;
    bool show_help = false;
};

/*
 @desc: Displays CLI options and parameters for the be utility
*/
void print_help() {
    std::cout << "be - Binary Entropy Utility\n\n"
              << "USAGE:\n"
              << "    be <filename> [OPTIONS]\n\n"
              << "OPTIONS:\n"
              << "    -p, --percentage         Display entropy as a percentage (0.0% to 100.0%)\n"
              << "    -b, --block-size <bytes> Calculate entropy in chunked block intervals (default: 1024)\n"
              << "    -h, --help               Display help menu\n";
}

/*
 @param: const std::vector<unsigned char>& buffer, Data buffer
 @desc: Calculates Shannon entropy value (0.0 to 8.0 bits per byte) for given byte buffer
*/
double calculate_shannon_entropy(const std::vector<unsigned char>& buffer) {
    if (buffer.empty()) return 0.0;

    std::vector<std::size_t> byte_counts(256, 0);
    for (unsigned char byte : buffer) {
        byte_counts[byte]++;
    }

    double entropy = 0.0;
    double total_bytes = static_cast<double>(buffer.size());

    for (std::size_t count : byte_counts) {
        if (count > 0) {
            double probability = static_cast<double>(count) / total_bytes;
            entropy -= probability * (std::log2(probability));
        }
    }

    return entropy;
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @param: Options& opts, Target Options structure
 @desc: Parses CLI arguments into Options structure
*/
bool parse_args(int argc, char* argv[], Options& opts) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-p" || arg == "--percentage") {
            opts.show_percentage = true;
        } else if (arg == "-b" || arg == "--block-size") {
            if (i + 1 < argc) {
                try {
                    opts.block_size = std::stoull(argv[++i]);
                    opts.block_mode = true;
                    if (opts.block_size == 0) return false;
                } catch (...) {
                    return false;
                }
            } else {
                return false;
            }
        } else if (opts.target_file.empty()) {
            opts.target_file = arg;
        } else {
            return false;
        }
    }

    return !opts.target_file.empty() || opts.show_help;
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @desc: Main entry point for binary entropy calculation tool
*/
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

    if (!fs::exists(opts.target_file)) {
        std::cerr << "Error: Target file not found.\n";
        return 1;
    }

    std::ifstream file(opts.target_file, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open input file.\n";
        return 1;
    }

    if (opts.block_mode) {
        std::vector<unsigned char> buffer(opts.block_size);
        std::size_t block_index = 0;

        while (file.read(reinterpret_cast<char*>(buffer.data()), opts.block_size) || file.gcount() > 0) {
            buffer.resize(file.gcount());
            double entropy = calculate_shannon_entropy(buffer);

            std::cout << "Block " << std::setw(6) << block_index << " (" 
                      << std::setw(8) << (block_index * opts.block_size) << " bytes): ";

            if (opts.show_percentage) {
                std::cout << std::fixed << std::setprecision(2) << ((entropy / 8.0) * 100.0) << "%\n";
            } else {
                std::cout << std::fixed << std::setprecision(4) << entropy << " bits/byte\n";
            }

            block_index++;
            buffer.resize(opts.block_size);
        }
    } else {
        std::vector<unsigned char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        double entropy = calculate_shannon_entropy(content);

        if (opts.show_percentage) {
            std::cout << std::fixed << std::setprecision(2) << ((entropy / 8.0) * 100.0) << "%\n";
        } else {
            std::cout << std::fixed << std::setprecision(4) << entropy << " bits/byte\n";
        }
    }

    return 0;
}