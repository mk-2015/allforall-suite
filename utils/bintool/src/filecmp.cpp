#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <filesystem>

#if defined(_WIN32)
    #include <windows.h>
    #include <lmcons.h>
#else
    #include <sys/stat.h>
    #include <pwd.h>
#endif

namespace fs = std::filesystem;

struct Options {
    std::string file1;
    std::string file2;
    bool compare_size = false;
    bool compare_user = false;
    bool compare_shortname = false;
    bool compare_pattern = false;
    bool case_sensitive = false;
    std::string pattern;
};

/*
 @param: char c, Character to convert
 @desc: Converts a single character to lower case
*/
char to_lower_char(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

/*
 @param: std::string str, Input string
 @desc: Converts an entire string to lower case
*/
std::string to_lower_str(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), to_lower_char);
    return str;
}

/*
 @param: const std::string& path, Path to the file
 @desc: Returns the owner username of a given file cross-platform
*/
std::string get_file_owner(const std::string& path) {
#if defined(_WIN32)
    DWORD size = UNLEN + 1;
    char username[UNLEN + 1];
    if (GetUserNameA(username, &size)) {
        return std::string(username);
    }
    return "";
#else
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        struct passwd* pw = getpwuid(info.st_uid);
        if (pw) {
            return std::string(pw->pw_name);
        }
    }
    return "";
#endif
}

/*
 @param: const std::string& path, Path to the file
 @param: std::vector<std::size_t>& positions, Vector to store pattern offsets
 @param: const std::string& pattern, Pattern string to find
 @param: bool case_sensitive, Toggle case sensitivity
 @desc: Scans a file and records starting offsets of all pattern matches
*/
bool extract_pattern_matches(const std::string& path, std::vector<std::size_t>& positions, const std::string& pattern, bool case_sensitive) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string search_pattern = pattern;

    if (!case_sensitive) {
        content = to_lower_str(content);
        search_pattern = to_lower_str(search_pattern);
    }

    if (search_pattern.empty()) {
        return true;
    }

    std::size_t pos = content.find(search_pattern, 0);
    while (pos != std::string::npos) {
        positions.push_back(pos);
        pos = content.find(search_pattern, pos + search_pattern.length());
    }

    return true;
}

/*
 @param: const std::string& f1, First file path
 @param: const std::string& f2, Second file path
 @param: bool case_sensitive, Toggle case sensitivity
 @desc: Performs exact content byte or case-insensitive string comparison
*/
bool compare_contents(const std::string& f1, const std::string& f2, bool case_sensitive) {
    std::ifstream file1(f1, std::ios::binary);
    std::ifstream file2(f2, std::ios::binary);

    if (!file1.is_open() || !file2.is_open()) {
        return false;
    }

    if (case_sensitive) {
        return std::equal(
            std::istreambuf_iterator<char>(file1), std::istreambuf_iterator<char>(),
            std::istreambuf_iterator<char>(file2), std::istreambuf_iterator<char>()
        );
    }

    std::string content1((std::istreambuf_iterator<char>(file1)), std::istreambuf_iterator<char>());
    std::string content2((std::istreambuf_iterator<char>(file2)), std::istreambuf_iterator<char>());

    return to_lower_str(content1) == to_lower_str(content2);
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @desc: Displays the utility usage guide and flags
*/
void print_help() {
    std::cout << "filecmp - Cross-Platform File Comparison Utility\n\n"
              << "USAGE:\n"
              << "    filecmp <file1> <file2> [OPTIONS]\n\n"
              << "DESCRIPTION:\n"
              << "    Compares two files based on specified flags or full content.\n"
              << "    Exits with code 0 if equal, or code 1 if different / on error.\n\n"
              << "OPTIONS:\n"
              << "    <file1> <file2>     Required paths to the files being compared.\n"
              << "    -s                  Compare file sizes (in bytes).\n"
              << "    -u                  Compare OS owner/user names.\n"
              << "    -S                  Compare base file names (e.g., dev/1 == proc/1).\n"
              << "    -p <pattern>        Compare positional occurrence offsets of <pattern>.\n"
              << "    -oC                 Enable case-sensitive comparisons (disabled by default).\n"
              << "    -h, --help          Display this help menu.\n";
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @param: Options& opts, Options struct to populate
 @desc: Parses command line parameters into options structure
*/
bool parse_args(int argc, char* argv[], Options& opts) {
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-s") {
            opts.compare_size = true;
        } else if (arg == "-u") {
            opts.compare_user = true;
        } else if (arg == "-S") {
            opts.compare_shortname = true;
        } else if (arg == "-oC") {
            opts.case_sensitive = true;
        } else if (arg == "-p") {
            opts.compare_pattern = true;
            if (i + 1 < argc) {
                opts.pattern = argv[++i];
            } else {
                return false;
            }
        } else if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        } else {
            positional.push_back(arg);
        }
    }

    if (positional.size() != 2) {
        return false;
    }

    opts.file1 = positional[0];
    opts.file2 = positional[1];
    return true;
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @desc: Main entry point for filecmp utility
*/
int main(int argc, char* argv[]) {
    Options opts;
    if (!parse_args(argc, argv, opts)) {
        std::cerr << "Usage: filecmp <file1> <file2> [-s] [-u] [-S] [-p <pattern>] [-oC]\n";
        return 1;
    }

    if (!fs::exists(opts.file1) || !fs::exists(opts.file2)) {
        std::cerr << "Error: One or both files do not exist.\n";
        return 1;
    }

    bool equal = true;

    if (opts.compare_size) {
        std::uintmax_t size1 = fs::file_size(opts.file1);
        std::uintmax_t size2 = fs::file_size(opts.file2);
        if (size1 != size2) {
            equal = false;
        }
    }

    if (equal && opts.compare_user) {
        std::string owner1 = get_file_owner(opts.file1);
        std::string owner2 = get_file_owner(opts.file2);
        
        if (!opts.case_sensitive) {
            owner1 = to_lower_str(owner1);
            owner2 = to_lower_str(owner2);
        }

        if (owner1 != owner2 || owner1.empty()) {
            equal = false;
        }
    }

    if (equal && opts.compare_shortname) {
        std::string name1 = fs::path(opts.file1).filename().string();
        std::string name2 = fs::path(opts.file2).filename().string();

        if (!opts.case_sensitive) {
            name1 = to_lower_str(name1);
            name2 = to_lower_str(name2);
        }

        if (name1 != name2) {
            equal = false;
        }
    }

    if (equal && opts.compare_pattern) {
        std::vector<std::size_t> pos1, pos2;
        bool read1 = extract_pattern_matches(opts.file1, pos1, opts.pattern, opts.case_sensitive);
        bool read2 = extract_pattern_matches(opts.file2, pos2, opts.pattern, opts.case_sensitive);

        if (!read1 || !read2 || pos1 != pos2) {
            equal = false;
        }
    }

    if (equal && !opts.compare_size && !opts.compare_user && !opts.compare_shortname && !opts.compare_pattern) {
        equal = compare_contents(opts.file1, opts.file2, opts.case_sensitive);
    }

    if (equal) {
        std::cout << "Files are EQUAL\n";
        return 0;
    } else {
        std::cout << "Files are DIFFERENT\n";
        return 1;
    }
}