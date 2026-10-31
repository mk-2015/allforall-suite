#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

struct Options {
    std::string oldText;
    std::string newText;
    std::string input;
    std::string output;
    bool hasSubstitution = false;
    bool ignoreCase = false;
    bool matchInside = false;
    bool useStdin = false;
    bool quiet = false;
    bool firstOnly = false;
    bool invert = false;
    bool countOnly = false;
    bool lineNumbers = false;
    bool regexOld = false;
    bool regexNew = false;

    enum class RangeType {
        All,
        Range,
        First,
        Last
    } rangeType = RangeType::All;

    size_t rangeStart = 1;
    size_t rangeEnd = 0;
};

static void usage()
{
    std::cout
        << "Usage: kp [OPTIONS] [INPUT [OUTPUT]]\n"
        << "\n"
        << "Kut and Paste\n"
        << "\n"
        << "Options modify input and optionally write to OUTPUT.\n"
        << "INPUT and OUTPUT are files; -stdin reads INPUT from stdin.\n"
        << "\n"
        << "OPTIONS:\n"
        << "  -s OLD NEW    Replace OLD with NEW\n"
        << "  -i            Case-insensitive matching\n"
        << "  -mi           Match OLD inside substrings\n"
        << "  -stdin        Read input from stdin\n"
        << "  -q            Quiet mode; requires OUTPUT\n"
        << "  -n            Replace only the first occurrence per line\n"
        << "  -g            Replace all occurrences (default)\n"
        << "  -v            Replace non-matching lines\n"
        << "  -c            Count replacements without replacing\n"
        << "  -l            Print line numbers of modified lines to stderr\n"
        << "  -f            Treat OLD as a regular expression\n"
        << "  -F            Use regex replacement formatting for NEW\n"
        << "  -R A:B        Operate on lines A through B, inclusive\n"
        << "  -r A          Operate on the first A lines\n"
        << "  -Rr A         Operate on the last A lines\n"
        << "  --help        Display this help\n";
}

static bool parseNumber(const std::string& value, size_t& result)
{
    if (value.empty())
        return false;

    try {
        size_t pos = 0;
        unsigned long long number = std::stoull(value, &pos);

        if (pos != value.size() || number == 0 ||
            number > static_cast<unsigned long long>(SIZE_MAX))
            return false;

        result = static_cast<size_t>(number);
        return true;
    } catch (...) {
        return false;
    }
}

static std::string escapeRegex(const std::string& value)
{
    static const std::string special = R"(\.^$|()[]{}*+?)";
    std::string result;
    result.reserve(value.size() * 2);

    for (char c : value) {
        if (special.find(c) != std::string::npos)
            result += '\\';
        result += c;
    }

    return result;
}

static std::string escapeReplacement(const std::string& value)
{
    std::string result;
    result.reserve(value.size() * 2);

    for (char c : value) {
        if (c == '$')
            result += '$';
        result += c;
    }

    return result;
}

static bool parseOptions(int argc, char** argv, Options& opt)
{
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            usage();
            return false;
        } else if (arg == "--") {
            while (++i < argc)
                positional.emplace_back(argv[i]);
            break;
        } else if (arg == "-s") {
            if (i + 2 >= argc) {
                std::cerr << "kp: -s requires OLD and NEW\n";
                return false;
            }
            opt.oldText = argv[++i];
            opt.newText = argv[++i];
            opt.hasSubstitution = true;
        } else if (arg == "-i") {
            opt.ignoreCase = true;
        } else if (arg == "-mi") {
            opt.matchInside = true;
        } else if (arg == "-stdin") {
            opt.useStdin = true;
        } else if (arg == "-q") {
            opt.quiet = true;
        } else if (arg == "-n") {
            opt.firstOnly = true;
        } else if (arg == "-g") {
            opt.firstOnly = false;
        } else if (arg == "-v") {
            opt.invert = true;
        } else if (arg == "-c") {
            opt.countOnly = true;
        } else if (arg == "-l") {
            opt.lineNumbers = true;
        } else if (arg == "-f") {
            opt.regexOld = true;
        } else if (arg == "-F") {
            opt.regexNew = true;
        } else if (arg == "-Rr" || arg == "-r") {
            if (++i >= argc) {
                std::cerr << "kp: " << arg << " requires a number\n";
                return false;
            }

            size_t number = 0;
            if (!parseNumber(argv[i], number)) {
                std::cerr << "kp: invalid line count for " << arg << '\n';
                return false;
            }

            if (arg == "-r") {
                opt.rangeType = Options::RangeType::First;
                opt.rangeEnd = number;
            } else {
                opt.rangeType = Options::RangeType::Last;
                opt.rangeStart = number;
            }
        } else if (arg == "-R") {
            if (++i >= argc) {
                std::cerr << "kp: -R requires A:B\n";
                return false;
            }

            const std::string range = argv[i];
            const size_t colon = range.find(':');

            if (colon == std::string::npos ||
                range.find(':', colon + 1) != std::string::npos ||
                !parseNumber(range.substr(0, colon), opt.rangeStart) ||
                !parseNumber(range.substr(colon + 1), opt.rangeEnd) ||
                opt.rangeStart > opt.rangeEnd) {
                std::cerr << "kp: invalid range; expected A:B with A <= B\n";
                return false;
            }

            opt.rangeType = Options::RangeType::Range;
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "kp: unknown option: " << arg << '\n';
            return false;
        } else {
            positional.push_back(arg);
        }
    }

    if (opt.regexNew && !opt.regexOld) {
        std::cerr << "kp: -F requires -f\n";
        return false;
    }

    if (!opt.hasSubstitution) {
        std::cerr << "kp: missing -s OLD NEW\n";
        return false;
    }

    if (opt.oldText.empty()) {
        std::cerr << "kp: OLD cannot be empty\n";
        return false;
    }

    if (opt.useStdin) {
        if (positional.size() > 1) {
            std::cerr << "kp: expected at most OUTPUT with -stdin\n";
            return false;
        }
        if (!positional.empty())
            opt.output = positional.front();
    } else {
        if (positional.empty() || positional.size() > 2) {
            std::cerr << "kp: expected INPUT [OUTPUT]\n";
            return false;
        }
        opt.input = positional[0];
        if (positional.size() == 2)
            opt.output = positional[1];
    }

    if (opt.quiet && opt.output.empty()) {
        std::cerr << "kp: -q requires OUTPUT\n";
        return false;
    }

    return true;
}

static bool lineSelected(size_t line, size_t total, const Options& opt)
{
    switch (opt.rangeType) {
        case Options::RangeType::All:
            return true;
        case Options::RangeType::Range:
            return line >= opt.rangeStart && line <= opt.rangeEnd;
        case Options::RangeType::First:
            return line <= opt.rangeEnd;
        case Options::RangeType::Last:
            return line > total - std::min(total, opt.rangeStart);
    }
    return false;
}

static size_t countMatches(const std::string& line,
                           const std::regex& pattern,
                           bool matchInside)
{
    if (!matchInside)
        return std::regex_match(line, pattern) ? 1u : 0u;

    size_t count = 0;
    for (std::sregex_iterator it(line.begin(), line.end(), pattern), end;
         it != end; ++it) {
        ++count;
    }
    return count;
}

static std::string makeReplacement(const Options& opt)
{
    return opt.regexNew ? opt.newText : escapeReplacement(opt.newText);
}

static std::string replaceLine(const std::string& line,
                               const std::regex& pattern,
                               const Options& opt)
{
    std::regex_constants::match_flag_type flags =
        std::regex_constants::format_default;

    if (opt.firstOnly)
        flags |= std::regex_constants::format_first_only;

    if (opt.matchInside)
        return std::regex_replace(line, pattern, makeReplacement(opt), flags);

    std::smatch match;
    if (!std::regex_match(line, match, pattern))
        return line;

    return match.format(makeReplacement(opt), flags);
}

static bool readInput(const Options& opt, std::string& content)
{
    std::istream* input = &std::cin;
    std::ifstream file;

    if (!opt.useStdin) {
        file.open(opt.input, std::ios::binary);
        if (!file) {
            std::cerr << "kp: cannot open input: " << opt.input << '\n';
            return false;
        }
        input = &file;
    }

    content.assign(std::istreambuf_iterator<char>(*input),
                   std::istreambuf_iterator<char>());

    if (input->bad()) {
        std::cerr << "kp: failed reading input\n";
        return false;
    }

    return true;
}

static bool writeOutput(const Options& opt, const std::string& result)
{
    if (opt.output.empty()) {
        if (!opt.quiet)
            std::cout.write(result.data(),
                            static_cast<std::streamsize>(result.size()));
        return static_cast<bool>(std::cout);
    }

    std::ofstream file(opt.output, std::ios::binary | std::ios::trunc);
    if (!file) {
        std::cerr << "kp: cannot open output: " << opt.output << '\n';
        return false;
    }

    file.write(result.data(), static_cast<std::streamsize>(result.size()));
    if (!file) {
        std::cerr << "kp: failed writing output: " << opt.output << '\n';
        return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    Options opt;

    if (!parseOptions(argc, argv, opt))
        return 1;

    std::regex pattern;

    try {
        const std::string expression =
            opt.regexOld ? opt.oldText : escapeRegex(opt.oldText);

        auto flags = std::regex_constants::ECMAScript;
        if (opt.ignoreCase)
            flags |= std::regex_constants::icase;

        pattern = std::regex(expression, flags);
    } catch (const std::regex_error& error) {
        std::cerr << "kp: invalid regular expression: "
                  << error.what() << '\n';
        return 1;
    }

    std::string content;
    if (!readInput(opt, content))
        return 1;

    const bool finalNewline =
        !content.empty() && content.back() == '\n';

    std::vector<std::string> lines;
    size_t start = 0;

    while (start < content.size()) {
        const size_t newline = content.find('\n', start);
        if (newline == std::string::npos) {
            lines.emplace_back(content.substr(start));
            start = content.size();
        } else {
            lines.emplace_back(content.substr(start, newline - start));
            start = newline + 1;
        }
    }

    const size_t totalLines = lines.size();
    size_t totalReplacements = 0;
    std::vector<size_t> modifiedLines;

    for (size_t i = 0; i < totalLines; ++i) {
        const size_t lineNumber = i + 1;

        if (!lineSelected(lineNumber, totalLines, opt))
            continue;

        const size_t matches =
            countMatches(lines[i], pattern, opt.matchInside);

        if (opt.invert) {
            if (matches != 0)
                continue;

            ++totalReplacements;
            if (!opt.countOnly) {
                lines[i] = opt.newText;
                modifiedLines.push_back(lineNumber);
            }
            continue;
        }

        if (matches == 0)
            continue;

        const size_t replacements =
            opt.matchInside ? (opt.firstOnly ? 1u : matches) : 1u;

        totalReplacements += replacements;

        if (!opt.countOnly) {
            lines[i] = replaceLine(lines[i], pattern, opt);
            modifiedLines.push_back(lineNumber);
        }
    }

    if (opt.countOnly) {
        if (!opt.quiet)
            std::cout << totalReplacements << '\n';
        return 0;
    }

    if (opt.lineNumbers) {
        for (size_t lineNumber : modifiedLines)
            std::cerr << lineNumber << '\n';
    }

    std::string result;
    for (size_t i = 0; i < lines.size(); ++i) {
        result += lines[i];
        if (i + 1 < lines.size() || finalNewline)
            result += '\n';
    }

    return writeOutput(opt, result) ? 0 : 1;
}
