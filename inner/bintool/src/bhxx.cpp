#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <filesystem>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <utility>
#include <system_error>
#include <re2/re2.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#define NS_ISATTY _isatty
#define NS_FILENO _fileno
#else
#include <unistd.h>
#define NS_ISATTY isatty
#define NS_FILENO(f) fileno(f)
#endif

namespace fs = std::filesystem;

enum class MatchMode {
    Fixed,
    BasicRegex,
    ExtendedRegex,
    Pcre2
};

struct MatchOptions {
    MatchMode mode = MatchMode::BasicRegex;
    bool ignore_case = false;
    bool invert_match = false;
    bool word_regexp = false;
    bool line_regexp = false;
    bool only_matching = false;
    bool count_only = false;
    bool files_with_matches = false;
    bool files_without_matches = false;
    bool quiet = false;
    bool suppress_errors = false;
    bool show_line_number = false;
    bool show_filename = false;
    bool explicit_show_filename = false;
    bool byte_offset = false;
    std::string color_mode = "auto";
    int max_count = -1;
    int context_before = 0;
    int context_after = 0;
    int context_surround = 0;
    bool recursive = false;
    bool dereference_recursive = false;
    std::vector<std::string> patterns;
    std::vector<std::string> include_patterns;
    std::vector<std::string> exclude_patterns;
    std::vector<std::string> exclude_dir_patterns;
    std::vector<std::string> paths;
};

struct MatchSpan {
    size_t start;
    size_t length;

    bool operator<(const MatchSpan& other) const {
        if (start != other.start) return start < other.start;
        return length < other.length;
    }

    bool operator==(const MatchSpan& other) const {
        return start == other.start && length == other.length;
    }
};

class IMatcher {
public:
    virtual ~IMatcher() = default;
    virtual std::vector<MatchSpan> match(const char* data, size_t size) = 0;
};

bool parse_strict_non_negative_int(const std::string& str, int& val) {
    if (str.empty()) return false;
    size_t idx = 0;
    try {
        long long parsed = std::stoll(str, &idx);
        if (idx != str.size() || parsed < 0 || parsed > std::numeric_limits<int>::max()) {
            return false;
        }
        val = static_cast<int>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

static std::string to_lower_str(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}

static bool is_word_char(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    return std::isalnum(uc) || c == '_';
}

static std::vector<MatchSpan> sort_and_deduplicate_spans(std::vector<MatchSpan>& spans) {
    if (spans.empty()) return {};
    std::sort(spans.begin(), spans.end());
    spans.erase(std::unique(spans.begin(), spans.end()), spans.end());
    return spans;
}

static std::vector<MatchSpan> flatten_spans_for_rendering(const std::vector<MatchSpan>& spans) {
    if (spans.empty()) return {};
    std::vector<MatchSpan> sorted = spans;
    std::sort(sorted.begin(), sorted.end());
    std::vector<MatchSpan> flattened;
    MatchSpan current = sorted[0];

    for (size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i].start <= current.start + current.length) {
            size_t new_end = std::max(current.start + current.length, sorted[i].start + sorted[i].length);
            current.length = new_end - current.start;
        } else {
            flattened.push_back(current);
            current = sorted[i];
        }
    }
    flattened.push_back(current);
    return flattened;
}

class CompositeMatcher : public IMatcher {
public:
    explicit CompositeMatcher(std::vector<std::unique_ptr<IMatcher>> matchers)
        : matchers_(std::move(matchers)) {}

    std::vector<MatchSpan> match(const char* data, size_t size) override {
        std::vector<MatchSpan> all_spans;
        for (const auto& m : matchers_) {
            auto spans = m->match(data, size);
            for (const auto& s : spans) {
                all_spans.push_back(s);
            }
        }
        return sort_and_deduplicate_spans(all_spans);
    }
private:
    std::vector<std::unique_ptr<IMatcher>> matchers_;
};

class FixedMatcher : public IMatcher {
public:
    FixedMatcher(const std::vector<std::string>& patterns, bool ignore_case, bool word_regexp, bool line_regexp)
        : patterns_(patterns), ignore_case_(ignore_case), word_regexp_(word_regexp), line_regexp_(line_regexp) {
        if (ignore_case_) {
            for (auto& p : patterns_) {
                p = to_lower_str(p);
            }
        }
    }

    std::vector<MatchSpan> match(const char* data, size_t size) override {
        std::vector<MatchSpan> spans;
        for (const auto& pat : patterns_) {
            if (pat.empty()) {
                if (line_regexp_) {
                    if (size == 0) {
                        spans.push_back({0, 0});
                    }
                } else {
                    spans.push_back({0, 0});
                }
                continue;
            }

            if (size == 0) continue;

            std::string_view text(data, size);
            std::string search_text;
            if (ignore_case_) {
                search_text = to_lower_str(std::string(text));
                text = search_text;
            }

            size_t pos = 0;
            while ((pos = text.find(pat, pos)) != std::string_view::npos) {
                size_t start = pos;
                size_t len = pat.size();
                bool valid = true;

                if (word_regexp_) {
                    bool left_ok = (start == 0 || !is_word_char(text[start - 1]));
                    bool right_ok = (start + len == text.size() || !is_word_char(text[start + len]));
                    if (!left_ok || !right_ok) valid = false;
                }

                if (line_regexp_) {
                    if (start != 0 || start + len != text.size()) valid = false;
                }

                if (valid) {
                    spans.push_back({start, len});
                }
                pos += std::max(size_t(1), len);
            }
        }
        return sort_and_deduplicate_spans(spans);
    }
private:
    std::vector<std::string> patterns_;
    bool ignore_case_;
    bool word_regexp_;
    bool line_regexp_;
};

static std::string translate_bre_to_ere(const std::string& bre) {
    std::string ere;
    bool escaped = false;
    for (size_t i = 0; i < bre.size(); ++i) {
        char c = bre[i];
        if (escaped) {
            if (c == '(' || c == ')' || c == '{' || c == '}' || c == '+' || c == '?' || c == '|') {
                ere.push_back(c);
            } else {
                ere.push_back('\\');
                ere.push_back(c);
            }
            escaped = false;
        } else {
            if (c == '\\') {
                escaped = true;
            } else if (c == '(' || c == ')' || c == '{' || c == '}') {
                ere.push_back('\\');
                ere.push_back(c);
            } else if (c == '+' || c == '?' || c == '|') {
                ere.push_back('\\');
                ere.push_back(c);
            } else {
                ere.push_back(c);
            }
        }
    }
    if (escaped) {
        ere.push_back('\\');
    }
    return ere;
}

class Re2Matcher : public IMatcher {
public:
    Re2Matcher(const std::string& pattern, MatchMode mode, bool ignore_case, bool word_regexp, bool line_regexp)
        : line_regexp_(line_regexp) {
        std::string pat = pattern;
        if (mode == MatchMode::BasicRegex) {
            pat = translate_bre_to_ere(pattern);
        }
        if (word_regexp) {
            pat = "\\b(" + pat + ")\\b";
        }
        if (line_regexp) {
            pat = "^(" + pat + ")$";
        }

        re2::RE2::Options options;
        options.set_case_sensitive(!ignore_case);
        options.set_posix_syntax(true);

        re_ = std::make_unique<re2::RE2>(pat, options);
        if (!re_->ok()) {
            throw std::runtime_error("RE2 compilation error: " + re_->error());
        }
    }

    std::vector<MatchSpan> match(const char* data, size_t size) override {
        std::vector<MatchSpan> spans;
        re2::StringPiece text(data, size);
        re2::StringPiece submatch;
        size_t offset = 0;

        while (offset <= size) {
            if (!re_->Match(text, offset, size, re2::RE2::UNANCHORED, &submatch, 1)) {
                break;
            }
            size_t start = submatch.data() - data;
            size_t len = submatch.size();
            spans.push_back({start, len});

            if (len == 0) {
                if (offset == size) break;
                offset++;
            } else {
                offset = start + std::max(size_t(1), len);
            }
        }
        return sort_and_deduplicate_spans(spans);
    }
private:
    std::unique_ptr<re2::RE2> re_;
    bool line_regexp_;
};

class Pcre2Matcher : public IMatcher {
public:
    Pcre2Matcher(const std::string& pattern, bool ignore_case, bool word_regexp, bool line_regexp)
        : re_(nullptr), match_data_(nullptr), line_regexp_(line_regexp) {
        std::string pat = pattern;
        if (word_regexp) {
            pat = "\\b(" + pat + ")\\b";
        }
        if (line_regexp) {
            pat = "^(" + pat + ")$";
        }

        int options = PCRE2_UCP;
        if (ignore_case) {
            options |= PCRE2_CASELESS;
        }

        int errornumber = 0;
        PCRE2_SIZE erroroffset = 0;
        re_ = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(pat.c_str()), PCRE2_ZERO_TERMINATED, options, &errornumber, &erroroffset, nullptr);
        if (!re_) {
            PCRE2_UCHAR buffer[256];
            pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
            throw std::runtime_error(std::string("PCRE2 compilation failed: ") + reinterpret_cast<char*>(buffer));
        }
        match_data_ = pcre2_match_data_create_from_pattern(re_, nullptr);
        if (!match_data_) {
            pcre2_code_free(re_);
            throw std::runtime_error("PCRE2 match data allocation failed");
        }
    }

    ~Pcre2Matcher() override {
        if (match_data_) pcre2_match_data_free(match_data_);
        if (re_) pcre2_code_free(re_);
    }

    std::vector<MatchSpan> match(const char* data, size_t size) override {
        std::vector<MatchSpan> spans;
        PCRE2_SIZE* ovector;
        size_t offset = 0;

        while (offset <= size) {
            int rc = pcre2_match(re_, reinterpret_cast<PCRE2_SPTR>(data), size, offset, 0, match_data_, nullptr);
            if (rc < 0) {
                if (rc == PCRE2_ERROR_NOMATCH) break;
                throw std::runtime_error("PCRE2 matching error code: " + std::to_string(rc));
            }

            ovector = pcre2_get_ovector_pointer(match_data_);
            size_t start = ovector[0];
            size_t end = ovector[1];
            spans.push_back({start, end - start});

            if (start == end) {
                if (offset == size) break;
                offset++;
            } else {
                offset = end;
            }
        }
        return sort_and_deduplicate_spans(spans);
    }
private:
    pcre2_code* re_;
    pcre2_match_data* match_data_;
    bool line_regexp_;
};

struct ContextQueueEntry {
    size_t line_number;
    size_t byte_offset;
    std::string line;
};

static bool should_use_color(const std::string& color_mode) {
    if (color_mode == "never") return false;
    if (color_mode == "always") return true;
    if (std::getenv("NO_COLOR") != nullptr) return false;
    return NS_ISATTY(NS_FILENO(stdout)) != 0;
}


static bool read_line_bytes(std::istream& in,
                            std::string& line,
                            size_t& bytes_consumed) {
    line.clear();
    bytes_consumed = 0;

    char c;
    bool got_any = false;
    bool ended_with_newline = false;

    while (in.get(c)) {
        got_any = true;
        ++bytes_consumed;

        if (c == '\n') {
            ended_with_newline = true;
            break;
        }

        line.push_back(c);
    }

    if (ended_with_newline && !line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    return got_any;
}


static void process_stream(
    std::istream& in,
    const std::string& filename,
    IMatcher& matcher,
    const MatchOptions& options,
    int& total_matches,
    bool& any_match_found,
    bool& error_occurred,
    bool& quiet_matched
) {
    std::string line;
    size_t line_number = 0;
    size_t byte_offset = 0;
    int matches_in_file = 0;

    bool file_matched = false;
    bool file_processing_failed = false;
    bool use_color = should_use_color(options.color_mode);

    size_t active_block_end = 0;
    size_t last_printed_line_number = 0;

    std::vector<ContextQueueEntry> before_queue;

    const size_t before_count = static_cast<size_t>(
        std::max(options.context_before, options.context_surround)
    );
    const size_t after_count = static_cast<size_t>(
        std::max(options.context_after, options.context_surround)
    );

    const bool context_enabled =
        before_count > 0 || after_count > 0;

    auto print_prefix = [&](size_t number,
                            size_t offset,
                            char separator) {
        if (options.show_filename) {
            std::cout << filename << separator;
        }
        if (options.show_line_number) {
            std::cout << number << separator;
        }
        if (options.byte_offset) {
            std::cout << offset << separator;
        }
    };

    auto print_context_line = [&](const ContextQueueEntry& entry) {
        if (entry.line_number <= last_printed_line_number) {
            return;
        }

        print_prefix(entry.line_number, entry.byte_offset, '-');
        std::cout << entry.line << '\n';
        last_printed_line_number = entry.line_number;
    };

    auto print_match_line = [&](size_t number,
                                size_t offset,
                                const std::string& text,
                                const std::vector<MatchSpan>& spans) {
        if (options.only_matching && !options.invert_match) {
            for (const auto& span : spans) {
                if (span.start > text.size() ||
                    span.length > text.size() - span.start) {
                    continue;
                }

                print_prefix(number, offset + span.start, ':');

                if (use_color) {
                    std::cout << "\033[31m"
                              << text.substr(span.start, span.length)
                              << "\033[0m\n";
                } else {
                    std::cout << text.substr(span.start, span.length)
                              << '\n';
                }
            }
            return;
        }

        print_prefix(number, offset, ':');

        if (use_color && !spans.empty() && !options.invert_match) {
            const auto render_spans = flatten_spans_for_rendering(spans);
            size_t cursor = 0;

            for (const auto& span : render_spans) {
                if (span.start > text.size() ||
                    span.length > text.size() - span.start) {
                    continue;
                }

                if (span.start < cursor) {
                    continue;
                }

                std::cout << text.substr(cursor, span.start - cursor);
                std::cout << "\033[31m"
                          << text.substr(span.start, span.length)
                          << "\033[0m";

                cursor = span.start + span.length;
            }

            std::cout << text.substr(cursor) << '\n';
        } else {
            std::cout << text << '\n';
        }
    };

    size_t bytes_consumed = 0;

    while (read_line_bytes(in, line, bytes_consumed)) {
        ++line_number;

        const size_t current_line_byte_offset = byte_offset;
        byte_offset += bytes_consumed;

        std::vector<MatchSpan> spans;
        bool matched = false;

        try {
            if (options.max_count != 0) {
                spans = matcher.match(line.data(), line.size());
                matched = !spans.empty();

                if (options.invert_match) {
                    matched = !matched;
                }
            }
        } catch (const std::exception& e) {
            if (!options.suppress_errors) {
                std::cerr << "nsgrep: " << filename
                          << ": " << e.what() << '\n';
            }

            error_occurred = true;
            file_processing_failed = true;
            break;
        }

        if (matched) {
            file_matched = true;
            any_match_found = true;
            ++matches_in_file;
            ++total_matches;

            if (options.quiet) {
                quiet_matched = true;
                return;
            }

            if (options.files_with_matches) {
                std::cout << filename << '\n';
                return;
            }

            if (!options.count_only) {
                const size_t before_start =
                    line_number > before_count
                        ? line_number - before_count
                        : 1;

                if (context_enabled &&
                    last_printed_line_number != 0 &&
                    before_start > last_printed_line_number + 1) {
                    std::cout << "--\n";
                }

                for (const auto& entry : before_queue) {
                    if (entry.line_number >= before_start &&
                        entry.line_number < line_number) {
                        print_context_line(entry);
                    }
                }
                before_queue.clear();

                if (line_number > last_printed_line_number) {
                    print_match_line(
                        line_number,
                        current_line_byte_offset,
                        line,
                        spans
                    );
                    last_printed_line_number = line_number;
                }

                const size_t proposed_end =
                    line_number + after_count;

                active_block_end =
                    std::max(active_block_end, proposed_end);
            }

            if (options.max_count >= 0 &&
                matches_in_file >= options.max_count) {
                break;
            }
        } else if (!options.count_only && context_enabled &&
                   line_number <= active_block_end) {
            ContextQueueEntry entry{
                line_number,
                current_line_byte_offset,
                line
            };
            print_context_line(entry);
        } else if (!options.count_only && before_count > 0) {
            before_queue.push_back({
                line_number,
                current_line_byte_offset,
                line
            });

            if (before_queue.size() > before_count) {
                before_queue.erase(before_queue.begin());
            }
        }
    }

    if (in.bad()) {
        if (!options.suppress_errors) {
            std::cerr << "nsgrep: " << filename
                      << ": input read error\n";
        }

        error_occurred = true;
        file_processing_failed = true;
    }

    if (!file_processing_failed) {
        if (options.files_without_matches && !file_matched) {
            std::cout << filename << '\n';
        }

        if (options.count_only) {
            if (options.show_filename) {
                std::cout << filename << ':';
            }
            std::cout << matches_in_file << '\n';
        }
    }
}


int run_nsgrep(const MatchOptions& options) {
    if (options.patterns.empty()) {
        std::cerr << "nsgrep: no pattern specified\n";
        return 2;
    }

    std::unique_ptr<IMatcher> matcher;
    try {
        if (options.patterns.size() == 1) {
            if (options.mode == MatchMode::Fixed) {
                matcher = std::make_unique<FixedMatcher>(options.patterns, options.ignore_case, options.word_regexp, options.line_regexp);
            } else if (options.mode == MatchMode::Pcre2) {
                matcher = std::make_unique<Pcre2Matcher>(options.patterns[0], options.ignore_case, options.word_regexp, options.line_regexp);
            } else {
                matcher = std::make_unique<Re2Matcher>(options.patterns[0], options.mode, options.ignore_case, options.word_regexp, options.line_regexp);
            }
        } else {
            std::vector<std::unique_ptr<IMatcher>> matchers;
            for (const auto& pat : options.patterns) {
                if (options.mode == MatchMode::Fixed) {
                    matchers.push_back(std::make_unique<FixedMatcher>(std::vector<std::string>{pat}, options.ignore_case, options.word_regexp, options.line_regexp));
                } else if (options.mode == MatchMode::Pcre2) {
                    matchers.push_back(std::make_unique<Pcre2Matcher>(pat, options.ignore_case, options.word_regexp, options.line_regexp));
                } else {
                    matchers.push_back(std::make_unique<Re2Matcher>(pat, options.mode, options.ignore_case, options.word_regexp, options.line_regexp));
                }
            }
            matcher = std::make_unique<CompositeMatcher>(std::move(matchers));
        }
    } catch (const std::exception& e) {
        if (!options.suppress_errors) {
            std::cerr << "nsgrep: " << e.what() << "\n";
        }
        return 2;
    }

    int total_matches = 0;
    bool any_match_found = false;
    bool error_occurred = false;
    bool quiet_matched = false;

    bool show_filename = options.show_filename;
    if (!options.explicit_show_filename && (options.paths.size() > 1 || options.recursive)) {
        show_filename = true;
    }
    MatchOptions effective_options = options;
    effective_options.show_filename = show_filename;

    if (effective_options.quiet) {
        effective_options.only_matching = false;
        effective_options.count_only = false;
        effective_options.files_with_matches = false;
        effective_options.files_without_matches = false;
    }

    if (effective_options.paths.empty()) {
        process_stream(std::cin, "(standard input)", *matcher, effective_options, total_matches, any_match_found, error_occurred, quiet_matched);
    } else {
        for (const auto& path : effective_options.paths) {
            if (quiet_matched) break;
            std::error_code ec;
            bool is_reg = fs::is_regular_file(path, ec);
            if (ec) {
                error_occurred = true;
                if (!effective_options.suppress_errors) {
                    std::cerr << "nsgrep: " << path << ": " << ec.message() << "\n";
                }
                continue;
            }

            if (is_reg) {
                std::ifstream file(path, std::ios::binary);
                if (!file) {
                    error_occurred = true;
                    if (!effective_options.suppress_errors) {
                        std::cerr << "nsgrep: " << path << ": Permission denied or unreadable\n";
                    }
                    continue;
                }
                process_stream(file, path, *matcher, effective_options, total_matches, any_match_found, error_occurred, quiet_matched);
            } else {
                bool is_dir = fs::is_directory(path, ec);
                if (ec) {
                    error_occurred = true;
                    if (!effective_options.suppress_errors) {
                        std::cerr << "nsgrep: " << path << ": " << ec.message() << "\n";
                    }
                    continue;
                }

                if (is_dir && effective_options.recursive) {
                    try {
                        fs::directory_options dir_opts = effective_options.dereference_recursive ? fs::directory_options::follow_directory_symlink : fs::directory_options::none;
                        auto it = fs::recursive_directory_iterator(path, dir_opts, ec);
                        auto end = fs::recursive_directory_iterator();
                        while (it != end && !ec) {
                            if (quiet_matched) break;
                            const auto& entry = *it;
                            std::error_code entry_ec;
                            bool is_regular = entry.is_regular_file(entry_ec);
                            if (entry_ec) {
                                error_occurred = true;
                                if (!effective_options.suppress_errors) {
                                    std::cerr << "nsgrep: " << entry.path().string() << ": " << entry_ec.message() << "\n";
                                }
                            } else if (is_regular) {
                                std::ifstream file(entry.path(), std::ios::binary);
                                if (file) {
                                    process_stream(file, entry.path().string(), *matcher, effective_options, total_matches, any_match_found, error_occurred, quiet_matched);
                                } else {
                                    error_occurred = true;
                                    if (!effective_options.suppress_errors) {
                                        std::cerr << "nsgrep: " << entry.path().string() << ": Permission denied or unreadable\n";
                                    }
                                }
                            }
                            it.increment(ec);
                        }
                        if (ec) {
                            error_occurred = true;
                            if (!effective_options.suppress_errors) {
                                std::cerr << "nsgrep: " << path << ": " << ec.message() << "\n";
                            }
                        }
                    } catch (const fs::filesystem_error& e) {
                        error_occurred = true;
                        if (!effective_options.suppress_errors) {
                            std::cerr << "nsgrep: " << e.what() << "\n";
                        }
                    }
                } else {
                    error_occurred = true;
                    if (!effective_options.suppress_errors) {
                        std::cerr << "nsgrep: " << path << ": No such file or directory\n";
                    }
                }
            }
        }
    }

    if (effective_options.quiet && (any_match_found || quiet_matched)) {
        return 0;
    }
    if (error_occurred) {
        return 2;
    }
    return any_match_found ? 0 : 1;
}

int main(int argc, char* argv[]) {
    MatchOptions options;
    bool double_dash_encountered = false;
    bool pattern_explicitly_set = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (double_dash_encountered) {
            if (!pattern_explicitly_set && options.patterns.empty()) {
                options.patterns.push_back(arg);
                pattern_explicitly_set = true;
            } else {
                options.paths.push_back(arg);
            }
            continue;
        }

        if (arg == "--") {
            double_dash_encountered = true;
            continue;
        }

        if (arg == "-i") {
            options.ignore_case = true;
        } else if (arg == "-v") {
            options.invert_match = true;
        } else if (arg == "-w") {
            options.word_regexp = true;
        } else if (arg == "-x") {
            options.line_regexp = true;
        } else if (arg == "-G") {
            options.mode = MatchMode::BasicRegex;
        } else if (arg == "-E") {
            options.mode = MatchMode::ExtendedRegex;
        } else if (arg == "-F") {
            options.mode = MatchMode::Fixed;
        } else if (arg == "-P") {
            options.mode = MatchMode::Pcre2;
        } else if (arg == "-n") {
            options.show_line_number = true;
        } else if (arg == "-h") {
            options.show_filename = false;
            options.explicit_show_filename = true;
        } else if (arg == "-H") {
            options.show_filename = true;
            options.explicit_show_filename = true;
        } else if (arg == "-o") {
            options.only_matching = true;
        } else if (arg == "-c") {
            options.count_only = true;
        } else if (arg == "-l") {
            options.files_with_matches = true;
        } else if (arg == "-L") {
            options.files_without_matches = true;
        } else if (arg == "-q") {
            options.quiet = true;
        } else if (arg == "-s") {
            options.suppress_errors = true;
        } else if (arg == "-b") {
            options.byte_offset = true;
        } else if (arg == "-r") {
            options.recursive = true;
            options.dereference_recursive = false;
        } else if (arg == "-R") {
            options.recursive = true;
            options.dereference_recursive = true;
        } else if (arg == "--color" || arg == "--color=auto" || arg == "--color=always" || arg == "--color=never") {
            if (arg == "--color") {
                options.color_mode = "always";
            } else {
                options.color_mode = arg.substr(8);
            }
        } else if (arg.rfind("--color", 0) == 0) {
            std::cerr << "nsgrep: invalid color option -- '" << arg << "'\n";
            return 2;
        } else if (arg == "-e") {
            if (i + 1 < argc) {
                options.patterns.push_back(argv[++i]);
                pattern_explicitly_set = true;
            } else {
                std::cerr << "nsgrep: option requires an argument -- 'e'\n";
                return 2;
            }
        } else if (arg.rfind("-m", 0) == 0 || arg == "-m") {
            std::string val;
            if (arg == "-m" && i + 1 < argc) {
                val = argv[++i];
            } else if (arg.rfind("-m", 0) == 0 && arg.size() > 2) {
                val = arg.substr(2);
            } else {
                std::cerr << "nsgrep: option requires an argument -- 'm'\n";
                return 2;
            }
            if (!parse_strict_non_negative_int(val, options.max_count)) {
                std::cerr << "nsgrep: invalid max count: " << val << "\n";
                return 2;
            }
        } else if (arg.rfind("-B", 0) == 0 || arg == "-B") {
            std::string val;
            if (arg == "-B" && i + 1 < argc) {
                val = argv[++i];
            } else if (arg.rfind("-B", 0) == 0 && arg.size() > 2) {
                val = arg.substr(2);
            } else {
                std::cerr << "nsgrep: option requires an argument -- 'B'\n";
                return 2;
            }
            if (!parse_strict_non_negative_int(val, options.context_before)) {
                std::cerr << "nsgrep: invalid context length: " << val << "\n";
                return 2;
            }
        } else if (arg.rfind("-A", 0) == 0 || arg == "-A") {
            std::string val;
            if (arg == "-A" && i + 1 < argc) {
                val = argv[++i];
            } else if (arg.rfind("-A", 0) == 0 && arg.size() > 2) {
                val = arg.substr(2);
            } else {
                std::cerr << "nsgrep: option requires an argument -- 'A'\n";
                return 2;
            }
            if (!parse_strict_non_negative_int(val, options.context_after)) {
                std::cerr << "nsgrep: invalid context length: " << val << "\n";
                return 2;
            }
        } else if (arg.rfind("-C", 0) == 0 || arg == "-C") {
            std::string val;
            if (arg == "-C" && i + 1 < argc) {
                val = argv[++i];
            } else if (arg.rfind("-C", 0) == 0 && arg.size() > 2) {
                val = arg.substr(2);
            } else {
                std::cerr << "nsgrep: option requires an argument -- 'C'\n";
                return 2;
            }
            if (!parse_strict_non_negative_int(val, options.context_surround)) {
                std::cerr << "nsgrep: invalid context length: " << val << "\n";
                return 2;
            }
        } else if (arg.rfind("-", 0) == 0) {
            std::cerr << "nsgrep: invalid option -- '" << arg << "'\n";
            return 2;
        } else {
            if (!pattern_explicitly_set && options.patterns.empty()) {
                options.patterns.push_back(arg);
                pattern_explicitly_set = true;
            } else {
                options.paths.push_back(arg);
            }
        }
    }

    if (options.invert_match) {
        options.only_matching = false;
    }

    return run_nsgrep(options);
}
