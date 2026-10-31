#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <filesystem>

namespace fs = std::filesystem;

struct Options {
    std::string input_file;
    std::string output_base;
    bool use_c_lang = false;
    bool show_help = false;
};

/*
 @param: const std::string& input, String to sanitize
 @desc: Replaces non-alphanumeric characters with underscores to create a valid C/C++ identifier
*/
std::string sanitize_identifier(std::string input) {
    for (char& c : input) {
        if (!std::isalnum(static_cast<unsigned char>(c))) {
            c = '_';
        }
    }
    if (!input.empty() && std::isdigit(static_cast<unsigned char>(input[0]))) {
        input = "_" + input;
    }
    return input;
}

/*
 @desc: Prints utility usage and available CLI arguments
*/
void print_help() {
    std::cout << "bin2cl - Binary to C/C++ Header Converter\n\n"
              << "USAGE:\n"
              << "    bin2cl <file> <file-to-output> [OPTIONS]\n\n"
              << "OPTIONS:\n"
              << "    -lc, --langc    Generate C header (.h) instead of C++ header (.hpp)\n"
              << "    -h,  --help     Display this help menu\n";
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @param: Options& opts, Options struct reference
 @desc: Parses CLI options for input file, output base name, and language flags
*/
bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> positionals;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-lc" || arg == "--langc") {
            opts.use_c_lang = true;
        } else if (arg[0] != '-') {
            positionals.push_back(arg);
        } else {
            return false;
        }
    }

    if (opts.show_help) {
        return true;
    }

    if (positionals.size() != 2) {
        return false;
    }

    opts.input_file = positionals[0];
    opts.output_base = positionals[1];
    return true;
}

/*
 @param: const std::string& input_path, Path to source binary file
 @param: const std::string& output_base, Base name for output file
 @param: bool use_c_lang, True for C output (.h), False for C++ (.hpp)
 @desc: Reads binary input and writes formatted const unsigned char array to target header
*/
bool convert_bin_to_header(const std::string& input_path, const std::string& output_base, bool use_c_lang) {
    std::ifstream in(input_path, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Error: Unable to open input file: " << input_path << "\n";
        return false;
    }

    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    std::string ext = use_c_lang ? ".h" : ".hpp";
    std::string output_path = output_base + ext;

    std::ofstream out(output_path);
    if (!out.is_open()) {
        std::cerr << "Error: Unable to create output file: " << output_path << "\n";
        return false;
    }

    std::string var_name = sanitize_identifier(fs::path(input_path).filename().string());
    std::string guard_name = sanitize_identifier(fs::path(output_path).filename().string()) + "_INCLUDED";
    std::transform(guard_name.begin(), guard_name.end(), guard_name.begin(), ::toupper);

    out << "#ifndef " << guard_name << "\n";
    out << "#define " << guard_name << "\n\n";

    if (use_c_lang) {
        out << "#include <stddef.h>\n\n";
        out << "static const unsigned char " << var_name << "[] = {\n";
    } else {
        out << "#include <cstddef>\n\n";
        out << "inline constexpr unsigned char " << var_name << "[] = {\n";
    }

    out << std::hex << std::setfill('0');
    size_t count = 0;
    for (size_t i = 0; i < buffer.size(); ++i) {
        if (count == 0) {
            out << "    ";
        }

        out << "0x" << std::setw(2) << static_cast<int>(buffer[i]);

        if (i + 1 < buffer.size()) {
            out << ", ";
        }

        count++;
        if (count == 12) {
            out << "\n";
            count = 0;
        }
    }

    if (count != 0) {
        out << "\n";
    }

    out << "};\n";
    out << "inline constexpr size_t " << var_name << "_len = " << std::dec << buffer.size() << ";\n\n";
    out << "#endif // " << guard_name << "\n";

    out.close();
    return true;
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @desc: Application entry point for bin2cl CLI converter
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

    if (!fs::exists(opts.input_file)) {
        std::cerr << "Error: Input file does not exist.\n";
        return 1;
    }

    if (!convert_bin_to_header(opts.input_file, opts.output_base, opts.use_c_lang)) {
        return 1;
    }

    return 0;
}