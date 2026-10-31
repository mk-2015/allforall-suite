
#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

enum class Mode {
    Default,
    Duplicates,
    AllRepeated,
    Unique,
    Group
};

enum class Method {
    None,
    Prepend,
    Separate,
    Append,
    Both
};

struct Options {
    Mode mode = Mode::Default;
    Method method = Method::Separate;

    bool mode_set = false;
    bool count = false;
    bool ignore_case = false;
    bool null_delimited = false;
    bool non_adjacent = false;

    std::size_t skip_fields = 0;
    std::size_t skip_chars = 0;
    std::size_t compare_chars =
        std::numeric_limits<std::size_t>::max();

    std::string input = "-";
    std::string output = "-";
};

struct Group {
    std::string key;
    std::string representative;
    std::size_t count = 0;
    std::vector<std::string> records;
};

static void help(std::ostream& out) {
    out <<
        "Usage: buniq [OPTION]... [INPUT [OUTPUT]]\n"
        "Filter matching records from INPUT, writing OUTPUT.\n"
        "Defaults: standard input and standard output.\n\n"
        "  -c                 prefix records with group count\n"
        "  -d                 one representative per duplicate group\n"
        "  -D                 all records from duplicate groups\n"
        "  --all-repeated[=METHOD]\n"
        "                     METHOD: none, prepend, separate\n"
        "  -u                 records belonging to singleton groups\n"
        "  --group[=METHOD]   output all groups\n"
        "                     METHOD: separate, prepend, append, both\n"
        "  -i                 ASCII case-insensitive comparison\n"
        "  -f N               skip N fields before comparison\n"
        "  -s N               skip N bytes after field skipping\n"
        "  -w N               compare at most N bytes after skips\n"
        "  -z                 use NUL as record delimiter\n"
        "  -aj                group non-adjacent records globally\n"
        "  -h, --help         show this help\n"
        "  --                 stop option parsing\n";
}

static bool parse_number(std::string_view text, std::size_t& value) {
    if (text.empty())
        return false;

    std::size_t result = 0;
    const char* begin = text.data();
    const char* end = begin + text.size();

    auto r = std::from_chars(begin, end, result);
    if (r.ec != std::errc{} || r.ptr != end)
        return false;

    value = result;
    return true;
}

static bool set_mode(
    Options& o, Mode mode, Method method, std::string& error)
{
    if (o.mode_set) {
        error = "output-selection options are mutually exclusive";
        return false;
    }

    o.mode_set = true;
    o.mode = mode;
    o.method = method;
    return true;
}

static bool parse_method(
    std::string_view value,
    bool all_repeated,
    Method& method)
{
    if (all_repeated) {
        if (value == "none") {
            method = Method::None;
            return true;
        }
        if (value == "prepend") {
            method = Method::Prepend;
            return true;
        }
        if (value == "separate") {
            method = Method::Separate;
            return true;
        }
    } else {
        if (value == "separate") {
            method = Method::Separate;
            return true;
        }
        if (value == "prepend") {
            method = Method::Prepend;
            return true;
        }
        if (value == "append") {
            method = Method::Append;
            return true;
        }
        if (value == "both") {
            method = Method::Both;
            return true;
        }
    }

    return false;
}

static bool parse_options(
    int argc,
    char** argv,
    Options& o,
    bool& show_help,
    std::string& error)
{
    bool end_options = false;
    std::vector<std::string> operands;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);

        if (!end_options && arg == "--") {
            end_options = true;
            continue;
        }

        if (!end_options && (arg == "--help" || arg == "-h")) {
            show_help = true;
            return true;
        }

        if (!end_options && arg == "-c") {
            o.count = true;
            continue;
        }
        if (!end_options && arg == "-d") {
            if (!set_mode(o, Mode::Duplicates, Method::None, error))
                return false;
            continue;
        }
        if (!end_options && arg == "-D") {
            if (!set_mode(o, Mode::AllRepeated, Method::None, error))
                return false;
            continue;
        }
        if (!end_options && arg == "-u") {
            if (!set_mode(o, Mode::Unique, Method::None, error))
                return false;
            continue;
        }
        if (!end_options && arg == "-i") {
            o.ignore_case = true;
            continue;
        }
        if (!end_options && arg == "-z") {
            o.null_delimited = true;
            continue;
        }
        if (!end_options && arg == "-aj") {
            o.non_adjacent = true;
            continue;
        }

        if (!end_options &&
            (arg == "--all-repeated" ||
             arg.starts_with("--all-repeated="))) {
            Method method = Method::None;

            if (arg != "--all-repeated") {
                auto value = arg.substr(
                    std::string_view("--all-repeated=").size());

                if (!parse_method(value, true, method)) {
                    error = "invalid --all-repeated method: " +
                            std::string(value);
                    return false;
                }
            }

            if (!set_mode(o, Mode::AllRepeated, method, error))
                return false;
            continue;
        }

        if (!end_options &&
            (arg == "--group" || arg.starts_with("--group="))) {
            Method method = Method::Separate;

            if (arg != "--group") {
                auto value = arg.substr(
                    std::string_view("--group=").size());

                if (!parse_method(value, false, method)) {
                    error = "invalid --group method: " +
                            std::string(value);
                    return false;
                }
            }

            if (!set_mode(o, Mode::Group, method, error))
                return false;
            continue;
        }

        // -f N, -s N, -w N, and attached forms such as -f2.
        if (!end_options && arg.size() >= 2 &&
            arg[0] == '-' &&
            (arg[1] == 'f' || arg[1] == 's' || arg[1] == 'w')) {

            const char option = arg[1];
            std::string_view value = arg.substr(2);

            if (value.empty()) {
                if (i + 1 >= argc) {
                    error = std::string("option -") + option +
                            " requires a number";
                    return false;
                }
                value = argv[++i];
            }

            std::size_t number = 0;
            if (!parse_number(value, number)) {
                error = std::string("invalid number for -") +
                        option + ": " + std::string(value);
                return false;
            }

            if (option == 'f') o.skip_fields = number;
            if (option == 's') o.skip_chars = number;
            if (option == 'w') o.compare_chars = number;
            continue;
        }

        // Support simple flag clusters such as -ci and -cz.
        if (!end_options && arg.size() > 1 &&
            arg[0] == '-' && arg[1] != '-') {
            bool valid = true;

            for (std::size_t j = 1; j < arg.size(); ++j) {
                switch (arg[j]) {
                    case 'c': o.count = true; break;
                    case 'i': o.ignore_case = true; break;
                    case 'z': o.null_delimited = true; break;
                    default: valid = false; break;
                }
                if (!valid) break;
            }

            if (valid) continue;

            error = "unknown option: " + std::string(arg);
            return false;
        }

        if (!end_options && arg.size() > 1 &&
            arg[0] == '-' && arg[1] == '-') {
            error = "unknown option: " + std::string(arg);
            return false;
        }

        operands.emplace_back(arg);
    }

    if (operands.size() > 2) {
        error = "too many file operands";
        return false;
    }

    if (!operands.empty()) o.input = operands[0];
    if (operands.size() == 2) o.output = operands[1];

    return true;
}

// Construct the comparison key. The original record is never modified.
static std::string make_key(
    std::string_view record,
    const Options& o)
{
    std::size_t pos = 0;

    // Skip fields, treating spaces and tabs as separators.
    for (std::size_t field = 0; field < o.skip_fields; ++field) {
        while (pos < record.size() &&
               (record[pos] == ' ' || record[pos] == '\t')) {
            ++pos;
        }

        if (pos == record.size())
            break;

        while (pos < record.size() &&
               record[pos] != ' ' && record[pos] != '\t') {
            ++pos;
        }
    }

    // Skip whitespace after the last skipped field too.
    if (o.skip_fields != 0) {
        while (pos < record.size() &&
               (record[pos] == ' ' || record[pos] == '\t')) {
            ++pos;
        }
    }

    const std::size_t remaining = record.size() - pos;
    pos += std::min(o.skip_chars, remaining);

    const std::size_t available = record.size() - pos;
    const std::size_t length = std::min(available, o.compare_chars);

    std::string key(record.substr(pos, length));

    if (o.ignore_case) {
        for (char& c : key) {
            auto byte = static_cast<unsigned char>(c);

            if (byte >= 'A' && byte <= 'Z')
                c = static_cast<char>(byte + ('a' - 'A'));
        }
    }

    return key;
}

// Reads binary-safe records. A final nonempty unterminated record is accepted.
static bool read_records(
    std::istream& in,
    char delimiter,
    std::vector<std::string>& records)
{
    std::string record;
    char c;

    while (in.get(c)) {
        if (c == delimiter) {
            records.push_back(std::move(record));
            record.clear();
        } else {
            record.push_back(c);
        }
    }

    if (!record.empty())
        records.push_back(std::move(record));

    return !in.bad();
}

static std::vector<Group> make_groups(
    const std::vector<std::string>& records,
    const Options& o)
{
    std::vector<Group> groups;
    groups.reserve(records.size());

    if (!o.non_adjacent) {
        for (const auto& record : records) {
            std::string key = make_key(record, o);

            if (groups.empty() || groups.back().key != key) {
                Group group;
                group.key = std::move(key);
                group.representative = record;
                group.count = 1;
                group.records.push_back(record);
                groups.push_back(std::move(group));
            } else {
                auto& group = groups.back();
                ++group.count;
                group.records.push_back(record);
            }
        }

        return groups;
    }

    // Map keys to indexes; vector order preserves first appearance.
    std::unordered_map<std::string, std::size_t> indexes;
    indexes.reserve(records.size());

    for (const auto& record : records) {
        std::string key = make_key(record, o);
        auto found = indexes.find(key);

        if (found == indexes.end()) {
            Group group;
            group.key = key;
            group.representative = record;
            group.count = 1;
            group.records.push_back(record);

            const std::size_t index = groups.size();
            groups.push_back(std::move(group));
            indexes.emplace(std::move(key), index);
        } else {
            auto& group = groups[found->second];
            ++group.count;
            group.records.push_back(record);
        }
    }

    return groups;
}

static bool selected(const Group& group, const Options& o) {
    switch (o.mode) {
        case Mode::Default:
        case Mode::Group:
            return true;
        case Mode::Duplicates:
        case Mode::AllRepeated:
            return group.count > 1;
        case Mode::Unique:
            return group.count == 1;
    }
    return false;
}

static bool emit_record(
    std::ostream& out,
    const std::string& record,
    const Group& group,
    const Options& o,
    char delimiter)
{
    if (o.count)
        out << group.count << '\t';

    out.write(record.data(),
              static_cast<std::streamsize>(record.size()));
    out.put(delimiter);

    return static_cast<bool>(out);
}

static bool emit_group(
    std::ostream& out,
    const Group& group,
    const Options& o,
    char delimiter)
{
    if (!selected(group, o))
        return true;

    if (o.mode == Mode::Default ||
        o.mode == Mode::Duplicates ||
        o.mode == Mode::Unique) {
        return emit_record(out, group.representative,
                           group, o, delimiter);
    }

    for (const auto& record : group.records) {
        if (!emit_record(out, record, group, o, delimiter))
            return false;
    }

    return true;
}

static bool write_empty_record(std::ostream& out, char delimiter) {
    out.put(delimiter);
    return static_cast<bool>(out);
}

static bool write_output(
    std::ostream& out,
    const std::vector<Group>& groups,
    const Options& o,
    char delimiter)
{
    std::vector<std::size_t> emitted;
    emitted.reserve(groups.size());

    for (std::size_t i = 0; i < groups.size(); ++i) {
        if (selected(groups[i], o))
            emitted.push_back(i);
    }

    const bool repeated = o.mode == Mode::AllRepeated;
    const bool grouped = o.mode == Mode::Group;

    if (repeated && o.method == Method::Prepend &&
        !emitted.empty()) {
        if (!write_empty_record(out, delimiter)) return false;
    }

    if (grouped &&
        (o.method == Method::Prepend || o.method == Method::Both) &&
        !emitted.empty()) {
        if (!write_empty_record(out, delimiter)) return false;
    }

    for (std::size_t j = 0; j < emitted.size(); ++j) {
        if (j > 0) {
            if (repeated && o.method == Method::Separate) {
                if (!write_empty_record(out, delimiter)) return false;
            }

            if (grouped && o.method == Method::Separate) {
                if (!write_empty_record(out, delimiter)) return false;
            }
        }

        if (!emit_group(out, groups[emitted[j]], o, delimiter))
            return false;
    }

    if (grouped &&
        (o.method == Method::Append || o.method == Method::Both) &&
        !emitted.empty()) {
        if (!write_empty_record(out, delimiter)) return false;
    }

    out.flush();
    return static_cast<bool>(out);
}

static bool same_existing_file(
    const std::string& input,
    const std::string& output)
{
    if (input == "-" || output == "-")
        return false;

    std::error_code ec;
    const bool same = fs::equivalent(input, output, ec);
    return !ec && same;
}

int main(int argc, char** argv) {
    Options options;
    bool show_help = false;
    std::string error;

    if (!parse_options(argc, argv, options, show_help, error)) {
        std::cerr << "buniq: " << error
                  << "\nTry 'buniq --help' for usage.\n";
        return 2;
    }

    if (show_help) {
        help(std::cout);
        return 0;
    }

    if (same_existing_file(options.input, options.output)) {
        std::cerr << "buniq: input and output refer to the same file\n";
        return 2;
    }

    std::ifstream input_file;
    std::istream* in = &std::cin;

    if (options.input != "-") {
        input_file.open(options.input, std::ios::binary);

        if (!input_file) {
            std::cerr << "buniq: cannot open input '"
                      << options.input << "'\n";
            return 1;
        }

        in = &input_file;
    }

    const char delimiter = options.null_delimited ? '\0' : '\n';

    std::vector<std::string> records;
    if (!read_records(*in, delimiter, records)) {
        std::cerr << "buniq: error reading input\n";
        return 1;
    }

    // Input is fully read before output is opened.
    std::ofstream output_file;
    std::ostream* out = &std::cout;

    if (options.output != "-") {
        output_file.open(
            options.output,
            std::ios::binary | std::ios::trunc);

        if (!output_file) {
            std::cerr << "buniq: cannot open output '"
                      << options.output << "'\n";
            return 1;
        }

        out = &output_file;
    }

    const auto groups = make_groups(records, options);

    if (!write_output(*out, groups, options, delimiter)) {
        std::cerr << "buniq: error writing output\n";
        return 1;
    }

    return 0;
}
