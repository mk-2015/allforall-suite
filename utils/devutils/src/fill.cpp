#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <new>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

constexpr std::uint64_t MAX_COMBINATIONS = 1'000'000;
constexpr int EXIT_USAGE = 2;
constexpr int EXIT_ERROR = 1;

class FillError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct Options {
    std::string input_template;
    std::unordered_map<std::string, std::vector<std::string>> fills;
    bool randomize = false;
    bool all = true;
    std::uint64_t count = 0;
    bool count_seen = false;
};

std::string trim(const std::string& s) {
    std::size_t begin = 0;
    while (begin < s.size() &&
           std::isspace(static_cast<unsigned char>(s[begin]))) {
        ++begin;
    }

    std::size_t end = s.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }

    return s.substr(begin, end - begin);
}

bool valid_name(const std::string& name) {
    if (name.empty()) {
        return false;
    }

    const unsigned char first = static_cast<unsigned char>(name[0]);
    if (!(std::isalpha(first) || first == '_')) {
        return false;
    }

    for (unsigned char c : name) {
        if (!(std::isalnum(c) || c == '_' || c == '-')) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> parse_values(const std::string& text) {
    std::vector<std::string> values;
    std::size_t i = 0;

    while (i < text.size() &&
           std::isspace(static_cast<unsigned char>(text[i]))) {
        ++i;
    }

    if (i >= text.size() || text[i] != '[') {
        throw FillError("Expected '[' before fill values.");
    }
    ++i;

    while (true) {
        while (i < text.size() &&
               std::isspace(static_cast<unsigned char>(text[i]))) {
            ++i;
        }

        if (i >= text.size()) {
            throw FillError("Missing closing ']' in fill values.");
        }

        if (text[i] == ']') {
            ++i;
            break;
        }

        std::string value;

        if (text[i] == '"' || text[i] == '\'') {
            const char quote = text[i++];

            bool closed = false;
            while (i < text.size()) {
                char c = text[i++];

                if (c == '\\') {
                    if (i >= text.size()) {
                        throw FillError(
                            "Trailing escape character in quoted value.");
                    }
                    value.push_back(text[i++]);
                } else if (c == quote) {
                    closed = true;
                    break;
                } else {
                    value.push_back(c);
                }
            }

            if (!closed) {
                throw FillError("Unterminated quoted value.");
            }

            while (i < text.size() &&
                   std::isspace(static_cast<unsigned char>(text[i]))) {
                ++i;
            }

            if (i < text.size() && text[i] != ',' && text[i] != ']') {
                throw FillError(
                    "Expected ',' or ']' after quoted value.");
            }
        } else {
            const std::size_t start = i;

            while (i < text.size() && text[i] != ',' && text[i] != ']') {
                ++i;
            }

            value = trim(text.substr(start, i - start));

            if (value.empty()) {
                throw FillError("Empty fill value.");
            }
        }

        values.push_back(std::move(value));

        if (i >= text.size()) {
            throw FillError("Missing closing ']' in fill values.");
        }

        if (text[i] == ',') {
            ++i;

            std::size_t next = i;
            while (next < text.size() &&
                std::isspace(static_cast<unsigned char>(text[next]))) {
                ++next;
            }

            if (next >= text.size() || text[next] == ']') {
                throw FillError("Trailing comma in fill values.");
            }

            continue;
        }

        if (text[i] == ']') {
            ++i;
            break;
        }

        throw FillError("Expected ',' or ']' in fill values.");
    }

    while (i < text.size() &&
           std::isspace(static_cast<unsigned char>(text[i]))) {
        ++i;
    }

    if (i != text.size()) {
        throw FillError("Unexpected characters after fill values.");
    }

    if (values.empty()) {
        throw FillError("A fill definition must contain at least one value.");
    }

    return values;
}

void print_help(const char* program) {
    std::cout
        << "Usage: " << program
        << " [OPTIONS] TEMPLATE --fill=NAME:[VALUES...]\n\n"
        << "Expand placeholders in a template using supplied values.\n\n"
        << "Options:\n"
        << "  -h, --help     Show this help message\n"
        << "  -a forall      Generate all combinations (default)\n"
        << "  -a NUM         Generate NUM combinations\n"
        << "  -r             Randomize output order; with -a NUM, select\n"
        << "                 NUM randomly ordered distinct combinations\n"
        << "  --             Stop processing options\n\n"
        << "Template syntax:\n"
        << "  Use <name> to mark a placeholder.\n\n"
        << "Fill syntax:\n"
        << "  --fill=NAME:[value1,value2,...]\n"
        << "  Values may be quoted with single or double quotes.\n\n"
        << "Examples:\n"
        << "  " << program
        << " '<first>.<last>@<service>' \\\n"
        << "    '--fill=first:[\"jessica\", \"alan\"]' \\\n"
        << "    '--fill=last:[\"jones\", \"becker\"]' \\\n"
        << "    '--fill=service:[\"gmail.com\", \"yahoo.com\"]'\n\n"
        << "  " << program
        << " -a 3 -r '<first>-<last>' \\\n"
        << "    '--fill=first:[jessica,alan]' \\\n"
        << "    '--fill=last:[jones,becker]'\n\n"
        << "Maximum raw combinations: " << MAX_COMBINATIONS << '\n';
}

std::vector<std::string> parse_template_names(
    const std::string& input_template) {

    std::vector<std::string> names;
    std::unordered_set<std::string> seen;

    for (std::size_t i = 0; i < input_template.size();) {
        if (input_template[i] == '<') {
            const std::size_t close = input_template.find('>', i + 1);

            if (close == std::string::npos) {
                throw FillError("Unclosed placeholder in template.");
            }

            const std::string name =
                input_template.substr(i + 1, close - i - 1);

            if (!valid_name(name)) {
                throw FillError("Invalid placeholder name: <" + name + ">.");
            }

            if (seen.insert(name).second) {
                names.push_back(name);
            }

            i = close + 1;
        } else if (input_template[i] == '>') {
            throw FillError("Unexpected '>' in template.");
        } else {
            ++i;
        }
    }

    if (names.empty()) {
        throw FillError("Template must contain at least one <placeholder>.");
    }

    return names;
}

std::string expand_template(
    const std::string& input_template,
    const std::vector<std::string>& names,
    const std::vector<std::string>& selected_values) {

    std::unordered_map<std::string, std::string> replacements;

    for (std::size_t i = 0; i < names.size(); ++i) {
        replacements[names[i]] = selected_values[i];
    }

    std::string result;
    result.reserve(input_template.size());

    for (std::size_t i = 0; i < input_template.size();) {
        if (input_template[i] == '<') {
            const std::size_t close = input_template.find('>', i + 1);
            const std::string name =
                input_template.substr(i + 1, close - i - 1);

            result += replacements.at(name);
            i = close + 1;
        } else {
            result.push_back(input_template[i]);
            ++i;
        }
    }

    return result;
}

void generate(
    const std::string& input_template,
    const std::vector<std::string>& names,
    const std::unordered_map<std::string, std::vector<std::string>>& fills,
    std::size_t index,
    std::vector<std::string>& selected_values,
    std::vector<std::string>& results,
    std::unordered_set<std::string>& unique_results) {

    if (index == names.size()) {
        std::string result =
            expand_template(input_template, names, selected_values);

        if (unique_results.insert(result).second) {
            results.push_back(std::move(result));
        }
        return;
    }

    const auto& values = fills.at(names[index]);

    for (const auto& value : values) {
        selected_values[index] = value;

        generate(
            input_template,
            names,
            fills,
            index + 1,
            selected_values,
            results,
            unique_results);
    }
}

std::uint64_t parse_count(const std::string& text) {
    if (text.empty()) {
        throw FillError("Missing combination count after -a.");
    }

    std::uint64_t value = 0;

    for (unsigned char c : text) {
        if (!std::isdigit(c)) {
            throw FillError("Invalid combination count: " + text + ".");
        }

        const unsigned digit = c - '0';

        if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / 10) {
            throw FillError("Combination count is too large.");
        }

        value = value * 10 + digit;
    }

    if (value == 0 || value > MAX_COMBINATIONS) {
        throw FillError(
            "-a count must be between 1 and " +
            std::to_string(MAX_COMBINATIONS) + ".");
    }

    return value;
}

Options parse_arguments(int argc, char* argv[]) {
    Options options;
    bool options_enabled = true;
    bool template_seen = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (options_enabled && (arg == "-h" || arg == "--help")) {
            print_help(argv[0]);
            std::exit(0);
        }

        if (options_enabled && arg == "--") {
            options_enabled = false;
            continue;
        }

        if (options_enabled && arg == "-r") {
            options.randomize = true;
            continue;
        }

        if (options_enabled && arg == "-a") {
            if (options.count_seen) {
                throw FillError("-a specified more than once.");
            }
            options.count_seen = true;

            if (i + 1 >= argc) {
                throw FillError("Missing argument after -a.");
            }

            const std::string count_arg = argv[++i];

            if (count_arg == "forall") {
                options.all = true;
            } else {
                options.all = false;
                options.count = parse_count(count_arg);
            }
            continue;
        }

        if (options_enabled && arg.rfind("--fill=", 0) == 0) {
            const std::string definition = arg.substr(7);
            const std::size_t colon = definition.find(':');

            if (colon == std::string::npos) {
                throw FillError(
                    "Invalid fill definition '" + arg +
                    "'. Expected --fill=NAME:[VALUES].");
            }

            const std::string name = definition.substr(0, colon);

            if (!valid_name(name)) {
                throw FillError("Invalid fill name: " + name + ".");
            }

            auto values = parse_values(definition.substr(colon + 1));

            if (options.fills.find(name) != options.fills.end()) {
                throw FillError("Duplicate fill definition: " + name + ".");
            }

            options.fills.emplace(name, std::move(values));
            continue;
        }

        if (options_enabled && !arg.empty() && arg[0] == '-') {
            throw FillError("Unknown option: " + arg + ".");
        }

        if (!template_seen) {
            options.input_template = arg;
            template_seen = true;
        } else {
            throw FillError("Unexpected extra argument: " + arg + ".");
        }
    }

    if (!template_seen) {
        throw FillError("Missing template.");
    }

    if (options.fills.empty()) {
        throw FillError("At least one --fill=NAME:[VALUES] is required.");
    }

    return options;
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        Options options = parse_arguments(argc, argv);
        const auto names = parse_template_names(options.input_template);

        for (const auto& name : names) {
            if (options.fills.find(name) == options.fills.end()) {
                throw FillError(
                    "No fill definition provided for placeholder: " + name + ".");
            }
        }

        for (const auto& entry : options.fills) {
            if (std::find(names.begin(), names.end(), entry.first) == names.end()) {
                throw FillError(
                    "Fill definition '" + entry.first +
                    "' is not used in the template.");
            }
        }

        std::uint64_t raw_combinations = 1;

        for (const auto& name : names) {
            const std::uint64_t size = options.fills.at(name).size();

            if (raw_combinations > MAX_COMBINATIONS / size) {
                throw FillError(
                    "Combination count exceeds the safety limit of " +
                    std::to_string(MAX_COMBINATIONS) + ".");
            }

            raw_combinations *= size;
        }

        std::vector<std::string> results;
        std::unordered_set<std::string> unique_results;
        std::vector<std::string> selected_values(names.size());

        results.reserve(static_cast<std::size_t>(raw_combinations));
        unique_results.reserve(static_cast<std::size_t>(raw_combinations));

        generate(
            options.input_template,
            names,
            options.fills,
            0,
            selected_values,
            results,
            unique_results);

        if (results.empty()) {
            throw FillError("No combinations were generated.");
        }

        if (!options.all && options.count > results.size()) {
            throw FillError(
                "Requested " + std::to_string(options.count) +
                " combinations, but only " +
                std::to_string(results.size()) + " unique expansion(s) exist.");
        }

        if (options.randomize) {
            std::random_device rd;
            std::mt19937 generator(rd());
            std::shuffle(results.begin(), results.end(), generator);
        }

        if (!options.all) {
            results.resize(static_cast<std::size_t>(options.count));
        }

        for (const auto& result : results) {
            std::cout << result << '\n';
        }

        return 0;
    } catch (const FillError& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_USAGE;
    } catch (const std::bad_alloc&) {
        std::cerr << "Error: Not enough memory to generate combinations.\n";
        return EXIT_ERROR;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_ERROR;
    }
}
