#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Options {
    std::string input_file;
    std::string pattern_text;
    bool hex_mode = false;
    bool ignore_case = false;
    bool invert_match = false;
    bool show_offsets = true;
    std::size_t context_bytes = 0;
    std::size_t max_matches = 0;
    bool show_help = false;
};

void print_help() {
    std::cout << "bgrep - Binary Pattern Search Utility\n\n"
              << "USAGE:\n"
              << "    bgrep [OPTIONS] <file> <pattern>\n"
              << "    bgrep [OPTIONS] -e <pattern> <file>\n\n"
              << "OPTIONS:\n"
              << "    -e, --expr <pattern>   Search expression to match\n"
              << "    -x, --hex              Search for a hex byte pattern (00ffaa or 00 ff aa)\n"
              << "    -i, --ignore-case      Case-insensitive text matching\n"
              << "    -n, --line-number      Print match offsets in decimal/hex\n"
              << "    -v, --invert-match     Print non-matching offsets instead of matches\n"
              << "    -C, --context <bytes>  Print a context window around each match\n"
              << "    -m, --max-count <n>   Stop after N matches\n"
              << "    -h, --help             Display this help menu\n";
}

bool parse_size(const std::string& value, std::size_t& out) {
    try {
        out = static_cast<std::size_t>(std::stoull(value));
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_hex_pattern(const std::string& input, std::vector<unsigned char>& out) {
    std::string cleaned;
    cleaned.reserve(input.size());

    for (char ch : input) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        cleaned += ch;
    }

    if (cleaned.empty()) {
        return false;
    }

    if (cleaned.size() >= 2 && cleaned.substr(0, 2) == "0x") {
        cleaned = cleaned.substr(2);
    }

    if (cleaned.size() % 2 != 0) {
        return false;
    }

    out.clear();
    for (std::size_t i = 0; i < cleaned.size(); i += 2) {
        std::string pair = cleaned.substr(i, 2);
        try {
            unsigned long value = std::stoul(pair, nullptr, 16);
            out.push_back(static_cast<unsigned char>(value));
        } catch (...) {
            return false;
        }
    }

    return !out.empty();
}

bool read_file_bytes(const std::string& path, std::vector<unsigned char>& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    out.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return true;
}

std::vector<unsigned char> make_casefolded(const std::vector<unsigned char>& bytes) {
    std::vector<unsigned char> folded;
    folded.reserve(bytes.size());
    for (unsigned char ch : bytes) {
        if (std::isalpha(ch)) {
            folded.push_back(static_cast<unsigned char>(std::tolower(ch)));
        } else {
            folded.push_back(ch);
        }
    }
    return folded;
}

std::string format_context_bytes(const std::vector<unsigned char>& bytes, std::size_t start, std::size_t length) {
    std::ostringstream oss;
    for (std::size_t i = start; i < start + length && i < bytes.size(); ++i) {
        if (i > start) {
            oss << ' ';
        }
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> positionals;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-e" || arg == "--expr") {
            if (i + 1 >= argc) {
                return false;
            }
            opts.pattern_text = argv[++i];
        } else if (arg == "-x" || arg == "--hex") {
            opts.hex_mode = true;
        } else if (arg == "-i" || arg == "--ignore-case") {
            opts.ignore_case = true;
        } else if (arg == "-n" || arg == "--line-number") {
            opts.show_offsets = true;
        } else if (arg == "-v" || arg == "--invert-match") {
            opts.invert_match = true;
        } else if (arg == "-C" || arg == "--context") {
            if (i + 1 >= argc || !parse_size(argv[++i], opts.context_bytes)) {
                return false;
            }
        } else if (arg == "-m" || arg == "--max-count") {
            if (i + 1 >= argc || !parse_size(argv[++i], opts.max_matches)) {
                return false;
            }
        } else if (arg[0] != '-') {
            positionals.push_back(arg);
        } else {
            return false;
        }
    }

    if (opts.show_help) {
        return true;
    }

    if (positionals.size() == 0) {
        return false;
    }

    if (positionals.size() >= 1) {
        opts.input_file = positionals[0];
    }

    if (positionals.size() >= 2 && opts.pattern_text.empty()) {
        opts.pattern_text = positionals[1];
    }

    if (opts.pattern_text.empty()) {
        return false;
    }

    return true;
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

    std::vector<unsigned char> file_bytes;
    if (!read_file_bytes(opts.input_file, file_bytes)) {
        std::cerr << "Error: unable to read input file.\n";
        return 1;
    }

    std::vector<unsigned char> pattern_bytes;
    if (opts.hex_mode) {
        if (!parse_hex_pattern(opts.pattern_text, pattern_bytes)) {
            std::cerr << "Error: invalid hex pattern.\n";
            return 1;
        }
    } else {
        pattern_bytes.assign(opts.pattern_text.begin(), opts.pattern_text.end());
        if (pattern_bytes.empty()) {
            std::cerr << "Error: empty search pattern.\n";
            return 1;
        }
    }

    std::vector<unsigned char> matching_file = file_bytes;
    std::vector<unsigned char> matching_pattern = pattern_bytes;
    if (opts.ignore_case && !opts.hex_mode) {
        matching_file = make_casefolded(file_bytes);
        matching_pattern = make_casefolded(pattern_bytes);
    }

    std::size_t file_size = matching_file.size();
    std::size_t pattern_size = matching_pattern.size();
    std::size_t matches = 0;
    bool found_any = false;

    for (std::size_t offset = 0; offset + pattern_size <= file_size; ++offset) {
        bool match = true;
        for (std::size_t i = 0; i < pattern_size; ++i) {
            if (matching_file[offset + i] != matching_pattern[i]) {
                match = false;
                break;
            }
        }

        bool report = match ^ opts.invert_match;
        if (!report) {
            continue;
        }

        if (opts.max_matches > 0 && matches >= opts.max_matches) {
            break;
        }

        found_any = true;
        ++matches;

        if (opts.show_offsets) {
            std::cout << "offset=0x" << std::hex << offset << " (" << std::dec << offset << " bytes)";
            if (opts.context_bytes > 0) {
                std::size_t start = offset > opts.context_bytes ? offset - opts.context_bytes : 0;
                std::size_t length = std::min(std::size_t{opts.context_bytes * 2 + pattern_size}, file_size - start);
                std::cout << " context=" << format_context_bytes(file_bytes, start, length);
            }
            std::cout << "\n";
        }
    }

    if (!found_any) {
        std::cout << "No matches found.\n";
        return 1;
    }

    return 0;
}
