#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <cctype>
#include <algorithm>
#include <filesystem>
#include <bitset>

namespace fs = std::filesystem;

struct Options {
    std::string input_file;
    std::string output_file;
    std::size_t cols = 16;
    std::size_t group_bytes = 2;
    std::size_t seek_offset = 0;
    std::size_t max_bytes = 0;   // 0 = read all
    bool reverse = false;        // -r: hex to binary
    bool plain = false;          // -p: plain hex output
    bool colorize = false;       // -C/--color: ANSI colorization by byte type
    bool uppercase = false;      // -u: uppercase hex letters
    bool decimal_offset = false; // -d: show offset in decimal instead of hex
    bool bit_dump = false;       // -b: binary digit dump (01010001)
    bool autoskip = false;       // -a: toggle autoskip repeat lines with '*'
    bool little_endian = false;  // -e: little-endian dump mode
    bool show_help = false;
};

/*
 @desc: Displays command-line usage and available options for the bhxx tool
*/
void print_help() {
    std::cout << "bhxx - Advanced Binary Hex Viewer Utility\n\n"
              << "USAGE:\n"
              << "    bhxx [input_file] [output_file] [OPTIONS]\n\n"
              << "OPTIONS:\n"
              << "    -a, --autoskip           Toggle autoskip: A single '*' replaces duplicate lines\n"
              << "    -b, --bit                Binary digit dump (incompatible with -p)\n"
              << "    -c, --cols <bytes>       Format <bytes> per line (default: 16, -b: 6)\n"
              << "    -d, --decimal            Show file offset in decimal instead of hex\n"
              << "    -e, --little-endian      Little-endian dump mode\n"
              << "    -g, --group <bytes>      Group output by <bytes> (default: 2, -e: 4)\n"
              << "    -l, --len <bytes>        Stop after <bytes>\n"
              << "    -p, --plain              Plain hex dump style\n"
              << "    -r, --reverse            Reverse operation: convert hex dump back to binary\n"
              << "    -s, --seek <offset>      Start dumping at byte offset\n"
              << "    -u, --uppercase          Use upper case hex letters\n"
              << "    -C, --color              Enable ANSI color coding (nulls, printable, high-bit)\n"
              << "    -h, --help               Display this help menu\n";
}

/*
 @param: const std::string& str, Input string value
 @param: std::size_t& val, Target size variable
 @desc: Safely converts string inputs to std::size_t numeric types
*/
bool parse_size(const std::string& str, std::size_t& val) {
    try {
        val = std::stoull(str);
        return true;
    } catch (...) {
        return false;
    }
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values array
 @param: Options& opts, Configuration parameter reference
 @desc: Parses CLI command line flags into target Options structure
*/
bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> positionals;
    bool cols_set = false;
    bool group_set = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-c" || arg == "--cols") {
            if (i + 1 < argc && parse_size(argv[++i], opts.cols)) { cols_set = true; } else return false;
        } else if (arg == "-g" || arg == "--group") {
            if (i + 1 < argc && parse_size(argv[++i], opts.group_bytes)) { group_set = true; } else return false;
        } else if (arg == "-s" || arg == "--seek") {
            if (i + 1 < argc && parse_size(argv[++i], opts.seek_offset)) {} else return false;
        } else if (arg == "-l" || arg == "--len") {
            if (i + 1 < argc && parse_size(argv[++i], opts.max_bytes)) {} else return false;
        } else if (arg == "-p" || arg == "--plain") {
            opts.plain = true;
        } else if (arg == "-r" || arg == "--reverse") {
            opts.reverse = true;
        } else if (arg == "-C" || arg == "--color") {
            opts.colorize = true;
        } else if (arg == "-u" || arg == "--uppercase") {
            opts.uppercase = true;
        } else if (arg == "-d" || arg == "--decimal") {
            opts.decimal_offset = true;
        } else if (arg == "-b" || arg == "--bit") {
            opts.bit_dump = true;
        } else if (arg == "-a" || arg == "--autoskip") {
            opts.autoskip = true;
        } else if (arg == "-e" || arg == "--little-endian") {
            opts.little_endian = true;
        } else if (arg[0] != '-') {
            positionals.push_back(arg);
        } else {
            return false;
        }
    }

    if (opts.bit_dump && !cols_set) opts.cols = 6;
    if (opts.little_endian && !group_set) opts.group_bytes = 4;

    if (positionals.size() >= 1) opts.input_file = positionals[0];
    if (positionals.size() >= 2) opts.output_file = positionals[1];

    return true;
}

/*
 @param: unsigned char c, Target byte value
 @desc: Maps byte types to corresponding ANSI color sequence strings
*/
std::string get_color_code(unsigned char c) {
    if (c == 0x00) return "\033[90m";            // Dark Gray for Nulls
    if (std::isprint(c)) return "\033[32m";      // Green for Printable ASCII
    if (c == 0xFF) return "\033[31m";            // Red for 0xFF
    return "\033[36m";                           // Cyan for Non-printable
}

/*
 @param: const Options& opts, Configuration parameter options
 @desc: Reads binary target stream and prints formatted hex view output
*/
bool run_hex_dump(const Options& opts) {
    std::ifstream in;
    std::istream* input_stream = &std::cin;

    if (!opts.input_file.empty()) {
        in.open(opts.input_file, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Error: Cannot open input file.\n";
            return false;
        }
        input_stream = &in;
    }

    if (opts.seek_offset > 0 && !opts.input_file.empty()) {
        in.seekg(opts.seek_offset, std::ios::beg);
    }

    std::ofstream out;
    std::ostream* output_stream = &std::cout;

    if (!opts.output_file.empty()) {
        out.open(opts.output_file);
        if (!out.is_open()) {
            std::cerr << "Error: Cannot open output file.\n";
            return false;
        }
        output_stream = &out;
    }

    std::vector<unsigned char> buffer(opts.cols);
    std::vector<unsigned char> prev_buffer(opts.cols);
    bool has_prev = false;
    bool in_autoskip = false;

    std::size_t current_offset = opts.seek_offset;
    std::size_t bytes_read_total = 0;

    if (opts.uppercase) *output_stream << std::uppercase;

    while (*input_stream) {
        std::size_t to_read = opts.cols;
        if (opts.max_bytes > 0 && (bytes_read_total + to_read) > opts.max_bytes) {
            to_read = opts.max_bytes - bytes_read_total;
        }
        if (to_read == 0) break;

        input_stream->read(reinterpret_cast<char*>(buffer.data()), to_read);
        std::streamsize bytes_in_line = input_stream->gcount();
        if (bytes_in_line == 0) break;

        // Autoskip logic handling (-a)
        if (opts.autoskip && has_prev && static_cast<std::size_t>(bytes_in_line) == opts.cols) {
            if (std::equal(buffer.begin(), buffer.end(), prev_buffer.begin())) {
                if (!in_autoskip) {
                    *output_stream << "*\n";
                    in_autoskip = true;
                }
                current_offset += bytes_in_line;
                bytes_read_total += bytes_in_line;
                continue;
            }
        }
        in_autoskip = false;
        has_prev = true;
        std::copy(buffer.begin(), buffer.end(), prev_buffer.begin());

        if (opts.plain) {
            for (std::streamsize i = 0; i < bytes_in_line; ++i) {
                *output_stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
            }
            *output_stream << "\n";
        } else {
            // Offset Column (-d for decimal, default hex)
            if (opts.decimal_offset) {
                *output_stream << std::dec << std::setw(8) << std::setfill('0') << current_offset << ": ";
            } else {
                *output_stream << std::hex << std::setw(8) << std::setfill('0') << current_offset << ": ";
            }

            // Binary bit dump mode (-b)
            if (opts.bit_dump) {
                for (std::size_t i = 0; i < opts.cols; ++i) {
                    if (i < static_cast<std::size_t>(bytes_in_line)) {
                        if (opts.colorize) *output_stream << get_color_code(buffer[i]);
                        *output_stream << std::bitset<8>(buffer[i]);
                        if (opts.colorize) *output_stream << "\033[0m";
                    } else {
                        *output_stream << "        ";
                    }

                    if (opts.group_bytes > 0 && (i + 1) % opts.group_bytes == 0 && i + 1 < opts.cols) {
                        *output_stream << " ";
                    }
                }
            } else {
                // Hex dump mode (with optional -e little-endian byte ordering)
                for (std::size_t i = 0; i < opts.cols; ++i) {
                    std::size_t byte_idx = i;
                    if (opts.little_endian && opts.group_bytes > 0) {
                        std::size_t group_start = (i / opts.group_bytes) * opts.group_bytes;
                        std::size_t offset_in_group = i % opts.group_bytes;
                        byte_idx = group_start + (opts.group_bytes - 1 - offset_in_group);
                    }

                    if (byte_idx < static_cast<std::size_t>(bytes_in_line)) {
                        if (opts.colorize) *output_stream << get_color_code(buffer[byte_idx]);
                        *output_stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[byte_idx]);
                        if (opts.colorize) *output_stream << "\033[0m";
                    } else {
                        *output_stream << "  ";
                    }

                    if (opts.group_bytes > 0 && (i + 1) % opts.group_bytes == 0 && i + 1 < opts.cols) {
                        *output_stream << " ";
                    }
                }
            }

            *output_stream << "  ";

            // ASCII Sidebar Column
            for (std::streamsize i = 0; i < bytes_in_line; ++i) {
                unsigned char c = buffer[i];
                if (opts.colorize) *output_stream << get_color_code(c);
                *output_stream << (std::isprint(c) ? static_cast<char>(c) : '.');
                if (opts.colorize) *output_stream << "\033[0m";
            }
            *output_stream << "\n";
        }

        current_offset += bytes_in_line;
        bytes_read_total += bytes_in_line;
    }

    return true;
}

/*
 @param: const Options& opts, Configuration parameter options
 @desc: Converts formatted hex dump input stream back to binary stream (-r)
*/
bool run_hex_reverse(const Options& opts) {
    std::ifstream in(opts.input_file);
    std::istream* input_stream = opts.input_file.empty() ? &std::cin : &in;
    
    std::ofstream out(opts.output_file, std::ios::binary);
    std::ostream* output_stream = opts.output_file.empty() ? &std::cout : &out;

    std::string line;
    while (std::getline(*input_stream, line)) {
        if (line.empty() || line[0] == '*') continue;

        std::size_t colon_pos = line.find(':');
        std::string hex_part = (colon_pos != std::string::npos) ? line.substr(colon_pos + 1) : line;

        std::string clean_hex = "";
        for (char ch : hex_part) {
            if (std::isxdigit(ch)) {
                clean_hex += ch;
            } else if (ch == ' ' && clean_hex.length() % 2 == 0) {
                continue;
            } else if (colon_pos != std::string::npos && clean_hex.length() >= 32) {
                break;
            }
        }

        for (std::size_t i = 0; i + 1 < clean_hex.length(); i += 2) {
            std::string byte_str = clean_hex.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(std::stoul(byte_str, nullptr, 16));
            output_stream->write(reinterpret_cast<char*>(&byte), 1);
        }
    }

    return true;
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument array
 @desc: Application main entry point for bhxx binary hex viewer utility
*/
int main(int argc, char* argv[]) {
    Options opts;

    if (!parse_args(argc, argv, opts) || opts.show_help) {
        print_help();
        return opts.show_help ? 0 : 1;
    }

    if (!opts.input_file.empty() && !fs::exists(opts.input_file)) {
        std::cerr << "Error: Input binary file does not exist.\n";
        return 1;
    }

    bool success = opts.reverse ? run_hex_reverse(opts) : run_hex_dump(opts);
    return success ? 0 : 1;
}