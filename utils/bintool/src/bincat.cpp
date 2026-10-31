#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cctype>
#include <bitset>
#include <filesystem>

namespace fs = std::filesystem;

struct Options {
    std::vector<std::string> input_files;
    std::string output_file;
    bool uppercase = false;   // -u: uppercase hex tags (<0x1E>)
    bool colorize = true;    // default enabled, -C / --no-color disables
    bool text_only = false;   // -t / --text-only: prints printable ASCII + \n only
    bool binary_mode = false; // -b / --binary: formats non-printables as <0b00000000>
    bool show_help = false;
};

/*
 @desc: Displays command-line usage and available options for the bincat tool
*/
void print_help() {
    std::cout << "bincat - Safe Binary Stream Viewer & Concatenator\n\n"
              << "USAGE:\n"
              << "    bincat [input_files...] [OPTIONS]\n\n"
              << "OPTIONS:\n"
              << "    -o, --output <file>    Direct output stream to a file target\n"
              << "    -b, --binary           Format escape tags as 8-bit binary (<0b00011110>)\n"
              << "    -u, --uppercase        Format hex escape tags in uppercase (<0x1E>)\n"
              << "    -C, --no-color         Disable ANSI color highlights for synthesized tags\n"
              << "    -t, --text-only        Suppress non-printable tags; print ASCII and \\n only\n"
              << "    -h, --help             Display this help menu\n";
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values array
 @param: Options& opts, Configuration parameter reference
 @desc: Parses command line options into the Options structure
*/
bool parse_args(int argc, char* argv[], Options& opts) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                opts.output_file = argv[++i];
            } else {
                return false;
            }
        } else if (arg == "-b" || arg == "--binary") {
            opts.binary_mode = true;
        } else if (arg == "-u" || arg == "--uppercase") {
            opts.uppercase = true;
        } else if (arg == "-C" || arg == "--no-color") {
            opts.colorize = false;
        } else if (arg == "-t" || arg == "--text-only") {
            opts.text_only = true;
        } else if (arg[0] != '-') {
            opts.input_files.push_back(arg);
        } else {
            return false;
        }
    }
    return true;
}

/*
 @param: unsigned char c, Raw input byte
 @param: const Options& opts, Configuration parameters
 @desc: Formats a byte into literal character, empty string, or colorized tag (<0xXX> / <0bXXXXXXXX>)
*/
std::string format_byte(unsigned char c, const Options& opts) {
    // Standard printable ASCII or newline
    if (std::isprint(c) || c == '\n') {
        return std::string(1, static_cast<char>(c));
    }

    // In text-only mode, non-printable characters are completely suppressed
    if (opts.text_only) {
        return "";
    }

    std::string tag;

    if (opts.binary_mode) {
        tag = "<0b" + std::bitset<8>(c).to_string() + ">";
    } else {
        tag = "<0x";
        char hex1 = (c >> 4) & 0x0F;
        char hex2 = c & 0x0F;

        auto to_hex_char = [opts](char val) -> char {
            if (val < 10) return '0' + val;
            return (opts.uppercase ? 'A' : 'a') + (val - 10);
        };

        tag += to_hex_char(hex1);
        tag += to_hex_char(hex2);
        tag += ">";
    }

    if (opts.colorize) {
        return "\033[34m" + tag + "\033[0m"; // Blue highlight for generated tags
    }
    return tag;
}

/*
 @param: std::istream& input_stream, Input stream source
 @param: std::ostream& output_stream, Output stream target
 @param: const Options& opts, Execution parameters
 @desc: Processes binary input stream byte-by-byte and writes formatted stream to output
*/
void process_stream(std::istream& input_stream, std::ostream& output_stream, const Options& opts) {
    char ch;
    while (input_stream.get(ch)) {
        unsigned char byte = static_cast<unsigned char>(ch);
        output_stream << format_byte(byte, opts);
    }
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument array
 @desc: Main entry point for bincat binary viewer utility
*/
int main(int argc, char* argv[]) {
    Options opts;

    if (!parse_args(argc, argv, opts) || opts.show_help) {
        print_help();
        return opts.show_help ? 0 : 1;
    }

    std::ofstream out_file;
    std::ostream* output_stream = &std::cout;

    if (!opts.output_file.empty()) {
        out_file.open(opts.output_file, std::ios::binary);
        if (!out_file.is_open()) {
            std::cerr << "Error: Cannot open output file.\n";
            return 1;
        }
        output_stream = &out_file;
    }

    if (opts.input_files.empty()) {
        process_stream(std::cin, *output_stream, opts);
    } else {
        for (const auto& file_path : opts.input_files) {
            if (!fs::exists(file_path)) {
                std::cerr << "Error: Input file does not exist: " << file_path << "\n";
                return 1;
            }
            std::ifstream in_file(file_path, std::ios::binary);
            if (!in_file.is_open()) {
                std::cerr << "Error: Cannot open input file: " << file_path << "\n";
                return 1;
            }
            process_stream(in_file, *output_stream, opts);
        }
    }

    return 0;
}