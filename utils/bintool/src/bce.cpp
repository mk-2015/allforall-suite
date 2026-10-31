#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <filesystem>

namespace fs = std::filesystem;

struct Options {
    std::string output_file;
    std::size_t size_in_bytes = 1024;
    double target_entropy = 8.0; // Max entropy by default
    bool show_help = false;
};

/*
 @desc: Displays CLI options and parameters for the bce utility
*/
void print_help() {
    std::cout << "bce - Binary Create Entropy Utility\n\n"
              << "USAGE:\n"
              << "    bce <output_file> [OPTIONS]\n\n"
              << "OPTIONS:\n"
              << "    -s, --size <bytes>       Target output file size in bytes (default: 1024)\n"
              << "    -e, --entropy <0.0-8.0>  Target entropy value scale from 0.0 to 8.0 (default: 8.0)\n"
              << "    -h, --help               Display help menu\n";
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @param: Options& opts, Target Options structure
 @desc: Parses CLI flags and parameters into Options structure
*/
bool parse_args(int argc, char* argv[], Options& opts) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-s" || arg == "--size") {
            if (i + 1 < argc) {
                try {
                    opts.size_in_bytes = std::stoull(argv[++i]);
                } catch (...) {
                    return false;
                }
            } else return false;
        } else if (arg == "-e" || arg == "--entropy") {
            if (i + 1 < argc) {
                try {
                    opts.target_entropy = std::stod(argv[++i]);
                    if (opts.target_entropy < 0.0 || opts.target_entropy > 8.0) return false;
                } catch (...) {
                    return false;
                }
            } else return false;
        } else if (opts.output_file.empty()) {
            opts.output_file = arg;
        } else {
            return false;
        }
    }

    return !opts.output_file.empty() || opts.show_help;
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @desc: Main execution entry point for bce tool
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

    std::ofstream out(opts.output_file, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Error: Unable to create output file.\n";
        return 1;
    }

    std::random_device rd;
    std::mt19937 gen(rd());

    // Proportion controls high-entropy random bytes vs repetitive zero bytes
    double random_ratio = opts.target_entropy / 8.0;
    std::uniform_int_distribution<int> byte_dist(0, 255);
    std::uniform_real_distribution<double> ratio_dist(0.0, 1.0);

    std::vector<unsigned char> buffer(opts.size_in_bytes);
    for (std::size_t i = 0; i < opts.size_in_bytes; ++i) {
        if (ratio_dist(gen) <= random_ratio) {
            buffer[i] = static_cast<unsigned char>(byte_dist(gen));
        } else {
            buffer[i] = 0x00; // Zero bias reduces total entropy
        }
    }

    out.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    out.close();

    std::cout << "Successfully generated " << opts.size_in_bytes << " bytes with ~" 
              << opts.target_entropy << " target entropy at: " << opts.output_file << "\n";

    return 0;
}