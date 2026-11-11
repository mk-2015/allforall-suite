#include <algorithm>
#include <cctype>
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <system_error>
#include <filesystem>

namespace fs = std::filesystem;

enum class CaseFormat {
    Auto,
    CamelCase,
    PascalCase,
    SnakeCase,
    KebabCase,
    ConstantCase,
    TitleCase,
    Lower,
    Upper
};

struct Options {
    CaseFormat from_format = CaseFormat::Auto;
    CaseFormat to_format = CaseFormat::PascalCase;
    std::string input = "-";
    std::string output = "-";
    bool show_help = false;
    bool list_formats = false;
};

static void print_help(const char* prog) {
    std::cout << "Usage: " << prog << " [OPTIONS] [INPUT [OUTPUT]]\n"
              << "Transform text casing between various naming conventions.\n\n"
              << "Options:\n"
              << "  -a FORMAT        Source text format (default: auto)\n"
              << "  -b FORMAT        Destination text format (required)\n"
              << "  --list           List all supported formats\n"
              << "  -h, --help       Show this help message and exit\n\n"
              << "Supported Formats:\n"
              << "  camel, CamelCase         lowerCamelCase\n"
              << "  pascal, PascalCase       UpperCamelCase\n"
              << "  snake, SnakeCase         snake_case\n"
              << "  kebab, KebabCase         kebab-case\n"
              << "  constant, ConstantCase   CONSTANT_CASE\n"
              << "  title, TitleCase         Title Case\n"
              << "  lower                    lowercase\n"
              << "  upper                    UPPERCASE\n"
              << "  auto                     Automatic tokenization\n";
}

static void list_supported_formats() {
    std::cout << "Supported Formats:\n"
              << "  - camel / CamelCase\n"
              << "  - pascal / PascalCase\n"
              << "  - snake / SnakeCase\n"
              << "  - kebab / KebabCase\n"
              << "  - constant / ConstantCase\n"
              << "  - title / TitleCase\n"
              << "  - lower\n"
              << "  - upper\n"
              << "  - auto\n";
}

static CaseFormat parse_format(std::string_view name) {
    std::string lower;
    lower.reserve(name.size());
    for (char c : name) {
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    // Remove non-alphanumeric for flexible matching (e.g., "snake_case" -> "snakecase")
    std::string clean;
    for (char c : lower) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            clean.push_back(c);
        }
    }

    if (clean == "camel" || clean == "camelcase") return CaseFormat::CamelCase;
    if (clean == "pascal" || clean == "pascalcase") return CaseFormat::PascalCase;
    if (clean == "snake" || clean == "snakecase") return CaseFormat::SnakeCase;
    if (clean == "kebab" || clean == "kebabcase") return CaseFormat::KebabCase;
    if (clean == "constant" || clean == "constantcase" || clean == "screamingsnake") return CaseFormat::ConstantCase;
    if (clean == "title" || clean == "titlecase") return CaseFormat::TitleCase;
    if (clean == "lower" || clean == "lowercase") return CaseFormat::Lower;
    if (clean == "upper" || clean == "uppercase") return CaseFormat::Upper;
    if (clean == "auto") return CaseFormat::Auto;

    return CaseFormat::Auto;
}

static bool parse_options(int argc, char** argv, Options& opt, std::string& error) {
    bool end_opts = false;
    std::vector<std::string> operands;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (!end_opts && arg == "--") {
            end_opts = true;
            continue;
        }
        if (!end_opts && (arg == "-h" || arg == "--help")) {
            opt.show_help = true;
            return true;
        }
        if (!end_opts && arg == "--list") {
            opt.list_formats = true;
            return true;
        }
        if (!end_opts && arg == "-a") {
            if (i + 1 >= argc) {
                error = "option -a requires a format argument";
                return false;
            }
            opt.from_format = parse_format(argv[++i]);
            continue;
        }
        if (!end_opts && arg.starts_with("-a=")) {
            opt.from_format = parse_format(arg.substr(3));
            continue;
        }
        if (!end_opts && arg == "-b") {
            if (i + 1 >= argc) {
                error = "option -b requires a format argument";
                return false;
            }
            opt.to_format = parse_format(argv[++i]);
            continue;
        }
        if (!end_opts && arg.starts_with("-b=")) {
            opt.to_format = parse_format(arg.substr(3));
            continue;
        }
        if (!end_opts && arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
            // Check for combined short flags if any
            error = "unknown option: " + arg;
            return false;
        }
        if (!end_opts && arg.size() > 1 && arg[0] == '-' && arg[1] == '-') {
            error = "unknown option: " + arg;
            return false;
        }
        operands.push_back(arg);
    }

    if (operands.size() > 2) {
        error = "too many file operands";
        return false;
    }

    if (!operands.empty()) opt.input = operands[0];
    if (operands.size() == 2) opt.output = operands[1];

    return true;
}

// Tokenize text into words
static std::vector<std::string> tokenize(std::string_view text) {
    std::vector<std::string> words;
    std::string current_word;

    auto push_word = [&]() {
        if (!current_word.empty()) {
            // convert word to lowercase
            for (char& c : current_word) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            words.push_back(std::move(current_word));
            current_word.clear();
        }
    };

    for (std::size_t i = 0; i < text.size(); ++i) {
        char c = text[i];

        if (c == '_' || c == '-' || std::isspace(static_cast<unsigned char>(c))) {
            push_word();
        } else if (std::isupper(static_cast<unsigned char>(c))) {
            // CamelCase / PascalCase boundary detection
            // If previous char was lowercase, or if next char is lowercase (acronym boundary)
            if (!current_word.empty() && 
                (std::islower(static_cast<unsigned char>(current_word.back())) ||
                 (i + 1 < text.size() && std::islower(static_cast<unsigned char>(text[i + 1]))))) {
                push_word();
            }
            current_word.push_back(c);
        } else if (std::isalnum(static_cast<unsigned char>(c))) {
            current_word.push_back(c);
        } else {
            // punctuation or symbol
            push_word();
        }
    }
    push_word();

    return words;
}

static std::string format_words(const std::vector<std::string>& words, CaseFormat format) {
    if (words.empty()) return "";

    std::string result;

    switch (format) {
        case CaseFormat::CamelCase: {
            for (std::size_t i = 0; i < words.size(); ++i) {
                const auto& w = words[i];
                if (w.empty()) continue;
                if (i == 0) {
                    result += w;
                } else {
                    result += static_cast<char>(std::toupper(static_cast<unsigned char>(w[0])));
                    result += w.substr(1);
                }
            }
            break;
        }
        case CaseFormat::PascalCase: {
            for (const auto& w : words) {
                if (w.empty()) continue;
                result += static_cast<char>(std::toupper(static_cast<unsigned char>(w[0])));
                result += w.substr(1);
            }
            break;
        }
        case CaseFormat::SnakeCase: {
            for (std::size_t i = 0; i < words.size(); ++i) {
                if (i > 0) result += '_';
                result += words[i];
            }
            break;
        }
        case CaseFormat::KebabCase: {
            for (std::size_t i = 0; i < words.size(); ++i) {
                if (i > 0) result += '-';
                result += words[i];
            }
            break;
        }
        case CaseFormat::ConstantCase: {
            for (std::size_t i = 0; i < words.size(); ++i) {
                if (i > 0) result += '_';
                for (char c : words[i]) {
                    result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                }
            }
            break;
        }
        case CaseFormat::TitleCase: {
            for (std::size_t i = 0; i < words.size(); ++i) {
                const auto& w = words[i];
                if (w.empty()) continue;
                if (i > 0) result += ' ';
                result += static_cast<char>(std::toupper(static_cast<unsigned char>(w[0])));
                result += w.substr(1);
            }
            break;
        }
        case CaseFormat::Lower: {
            for (const auto& w : words) {
                result += w;
            }
            break;
        }
        case CaseFormat::Upper: {
            for (const auto& w : words) {
                for (char c : w) {
                    result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                }
            }
            break;
        }
        case CaseFormat::Auto: {
            // Default to SnakeCase if unspecified
            for (std::size_t i = 0; i < words.size(); ++i) {
                if (i > 0) result += '_';
                result += words[i];
            }
            break;
        }
    }

    return result;
}

int main(int argc, char** argv) {
    Options opt;
    std::string error;

    if (!parse_options(argc, argv, opt, error)) {
        std::cerr << "textformat: " << error << "\nTry 'textformat --help' for usage.\n";
        return 2;
    }

    if (opt.show_help) {
        print_help(argv[0]);
        return 0;
    }

    if (opt.list_formats) {
        list_supported_formats();
        return 0;
    }

    // Read input
    std::istream* in = &std::cin;
    std::ifstream fin;
    if (opt.input != "-") {
        fin.open(opt.input, std::ios::binary);
        if (!fin) {
            std::cerr << "textformat: cannot open input '" << opt.input << "'\n";
            return 1;
        }
        in = &fin;
    }

    std::string content((std::istreambuf_iterator<char>(*in)), std::istreambuf_iterator<char>());

    // Tokenize words
    auto words = tokenize(content);
    if (words.empty() && !content.empty()) {
        // If content isn't empty but tokenize produced nothing, keep content as single word
        words.push_back(content);
    }

    std::string output_text = format_words(words, opt.to_format);

    // Write output
    std::ostream* out = &std::cout;
    std::ofstream fout;
    if (opt.output != "-") {
        fout.open(opt.output, std::ios::binary | std::ios::trunc);
        if (!fout) {
            std::cerr << "textformat: cannot open output '" << opt.output << "'\n";
            return 1;
        }
        out = &fout;
    }

    *out << output_text;
    if (!content.empty() && content.back() == '\n' && (output_text.empty() || output_text.back() != '\n')) {
        *out << '\n';
    }

    return 0;
}
