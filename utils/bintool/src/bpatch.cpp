#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

struct Options {
    std::string target_file;
    std::string patch_hex_str;
    std::size_t offset = 0;
    bool insert_mode = false;      // --insert / -i flag
    bool add_surrounds = false;    // --surround / -s flag
    bool dry_run = false;          // --dry-run / -d flag
    bool show_help = false;
};

/*
 @param: const std::string& input, Escaped hex string (e.g. "\x00\xAE\x41")
 @param: std::vector<unsigned char>& out_bytes, Target byte vector
 @desc: Parses escaped hex sequences (\xHH) into raw byte array
*/
bool parse_escaped_hex(const std::string& input, std::vector<unsigned char>& out_bytes) {
    out_bytes.clear();
    std::size_t i = 0;
    while (i < input.length()) {
        if (i + 3 < input.length() && input[i] == '\\' && (input[i + 1] == 'x' || input[i + 1] == 'X')) {
            std::string hex_byte = input.substr(i + 2, 2);
            try {
                unsigned char byte = static_cast<unsigned char>(std::stoul(hex_byte, nullptr, 16));
                out_bytes.push_back(byte);
                i += 4;
            } catch (...) {
                return false;
            }
        } else {
            out_bytes.push_back(static_cast<unsigned char>(input[i]));
            i++;
        }
    }
    return !out_bytes.empty();
}

/*
 @desc: Prints usage menu and available command-line flags for bpatch tool
*/
void print_help() {
    std::cout << "bpatch - Binary Patching Utility\n\n"
              << "USAGE:\n"
              << "    bpatch <file> <hex> [OPTIONS]\n\n"
              << "OPTIONS:\n"
              << "    -o, --offset <offset>    Byte offset to apply patch (default: 0)\n"
              << "    -s, --surround           Embed marker banners around modified target bytes\n"
              << "    -i, --insert             Insert bytes at offset instead of overwriting existing data\n"
              << "    -d, --dry-run            Simulate patching operation without modifying target file\n"
              << "    -h, --help               Display this help menu\n";
}

/*
 @param: const std::string& str, String value to parse
 @param: std::size_t& val, Target numerical variable
 @desc: Converts string argument into numerical size_t type safely
*/
bool parse_offset_val(const std::string& str, std::size_t& val) {
    try {
        val = std::stoull(str);
        return true;
    } catch (...) {
        return false;
    }
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument array
 @param: Options& opts, Target Options structure reference
 @desc: Parses CLI flags and positional inputs into Options structure
*/
bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> positionals;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-o" || arg == "--offset") {
            if (i + 1 < argc && parse_offset_val(argv[++i], opts.offset)) {} else return false;
        } else if (arg == "-s" || arg == "--surround") {
            opts.add_surrounds = true;
        } else if (arg == "-i" || arg == "--insert") {
            opts.insert_mode = true;
        } else if (arg == "-d" || arg == "--dry-run") {
            opts.dry_run = true;
        } else if (arg[0] != '-') {
            positionals.push_back(arg);
        } else {
            return false;
        }
    }

    if (opts.show_help) return true;
    if (positionals.size() != 2) return false;

    opts.target_file = positionals[0];
    opts.patch_hex_str = positionals[1];
    return true;
}

/*
 @param: const Options& opts, Configuration parameter options
 @desc: Executes binary file byte replacement or insertion based on offset and mode flags
*/
bool apply_binary_patch(const Options& opts) {
    std::vector<unsigned char> patch_bytes;
    if (!parse_escaped_hex(opts.patch_hex_str, patch_bytes)) {
        std::cerr << "Error: Invalid hex pattern sequence.\n";
        return false;
    }

    if (opts.add_surrounds) {
        std::string pre_marker = " --EDIT-> ";
        std::string post_marker = " <-EDIT-- ";
        std::vector<unsigned char> final_payload;

        final_payload.insert(final_payload.end(), pre_marker.begin(), pre_marker.end());
        final_payload.insert(final_payload.end(), patch_bytes.begin(), patch_bytes.end());
        final_payload.insert(final_payload.end(), post_marker.begin(), post_marker.end());

        patch_bytes = final_payload;
    }

    std::ifstream in(opts.target_file, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Error: Unable to open target file for reading.\n";
        return false;
    }

    std::vector<unsigned char> file_data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    if (opts.offset > file_data.size()) {
        std::cerr << "Error: Specified offset exceeds target file size.\n";
        return false;
    }

    if (opts.insert_mode) {
        file_data.insert(file_data.begin() + opts.offset, patch_bytes.begin(), patch_bytes.end());
    } else {
        std::size_t required_size = opts.offset + patch_bytes.size();
        if (required_size > file_data.size()) {
            file_data.resize(required_size, 0x00);
        }
        std::copy(patch_bytes.begin(), patch_bytes.end(), file_data.begin() + opts.offset);
    }

    if (opts.dry_run) {
        std::cout << "[DRY-RUN] Successfully simulated patch at offset " << opts.offset 
                  << " (" << patch_bytes.size() << " bytes modified/inserted).\n";
        return true;
    }

    std::ofstream out(opts.target_file, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "Error: Unable to open target file for writing.\n";
        return false;
    }

    out.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());
    out.close();

    return true;
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument array
 @desc: Application entry point for bpatch utility
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
        std::cerr << "Error: Target binary file does not exist.\n";
        return 1;
    }

    if (!apply_binary_patch(opts)) {
        return 1;
    }

    return 0;
}