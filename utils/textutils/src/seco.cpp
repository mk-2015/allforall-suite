#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <ranges>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

// Cross-platform clipboard writer
bool write_to_clipboard(const std::string &text) {
#if defined(_WIN32)
  FILE *pipe = _popen("clip", "w");
#elif defined(__APPLE__)
  FILE *pipe = popen("pbcopy", "w");
#else
  // Try xclip first, then wl-copy for Wayland
  FILE *pipe = popen(
      "xclip -selection clipboard 2>/dev/null || wl-copy 2>/dev/null", "w");
#endif

  if (!pipe)
    return false;
  fwrite(text.data(), 1, text.size(), pipe);

#if defined(_WIN32)
  return _pclose(pipe) == 0;
#else
  return pclose(pipe) == 0;
#endif
}

// ANSI escape codes for URL highlighting
const std::string ANSI_RED = "\033[31m";
const std::string ANSI_RESET = "\033[0m";

std::string highlight_urls(const std::string &line) {
  // Simple URL regex matching http:// or https://
  std::regex url_regex(R"(https?://[^\s]+)");
  return std::regex_replace(line, url_regex, ANSI_RED + "$&" + ANSI_RESET);
}

void print_help() {
  std::cout
      << "Usage: seco [OPTIONS] [QUERY]\n"
      << "Search and Copy utility\n\n"
      << "Options:\n"
      << "  -F FILE       : Copy output to FILE and not clipboard.\n"
      << "  -S            : Output to stdout and not clipboard (overrides -F)\n"
      << "  -C            : Makes Query Case-sensitive\n"
      << "  -R            : Add Regex (Builtin C++ Regex Library)\n"
      << "  -D QUERY      : Disinclude results that contain QUERY\n"
      << "  -T TEMPL REPL : When result contains TEMPL replace it with REPL\n"
      << "  -U            : Include ONLY results that ARE NOT QUERY (Invert "
         "Match)\n"
      << "  -u            : Highlight URLs in RED\n"
      << "  -E            : Count outputted Lines, Characters and words in "
         "`wc` format\n"
      << "  -h            : Show this help message\n";
}

int main(int argc, char *argv[]) {
  // Options configuration
  std::string query = "";
  std::string output_file = "";
  bool to_stdout = false;
  bool case_sensitive = false;
  bool use_regex = false;
  std::string disinclude_query = "";
  bool has_disinclude = false;
  bool has_replace = false;
  std::string templ = "";
  std::string repl = "";
  bool invert_match = false;
  bool highlight_url_flag = false;
  bool count_stats = false;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-S") {
      to_stdout = true;
    } else if (arg == "-C") {
      case_sensitive = true;
    } else if (arg == "-R") {
      use_regex = true;
    } else if (arg == "-U") {
      invert_match = true;
    } else if (arg == "-u") {
      highlight_url_flag = true;
    } else if (arg == "-E") {
      count_stats = true;
    } else if (arg == "-F" && i + 1 < argc) {
      output_file = argv[++i];
    } else if (arg == "-D" && i + 1 < argc) {
      disinclude_query = argv[++i];
      has_disinclude = true;
    } else if (arg == "-T" && i + 2 < argc) {
      templ = argv[++i];
      repl = argv[++i];
      has_replace = true;
    } else if (arg == "-h" || arg == "--help") {
      print_help();
      return 0;
    } else if (arg[0] != '-') {
      query = arg;
    }
  }

  // Helper lambda for string searching
  auto matches_query = [&](const std::string &text, const std::string &q) {
    if (q.empty())
      return true;
    if (use_regex) {
      try {
        auto flags =
            case_sensitive
                ? std::regex_constants::awk
                : (std::regex_constants::awk | std::regex_constants::icase);
        std::regex rx(q, flags);
        return std::regex_search(text, rx);
      } catch (...) {
        std::cerr << "Error: Invalid regular expression.\n";
        return false;
      }
    } else {
      if (case_sensitive) {
        return text.find(q) != std::string::npos;
      } else {
        std::string t_lower = text;
        std::string q_lower = q;
        std::ranges::transform(t_lower, t_lower.begin(), ::tolower);
        std::ranges::transform(q_lower, q_lower.begin(), ::tolower);
        return t_lower.find(q_lower) != std::string::npos;
      }
    }
  };

  std::string line;
  std::stringstream accumulated_output;
  size_t line_count = 0;
  size_t word_count = 0;
  size_t char_count = 0;

  // Read from standard input line by line
  while (std::getline(std::cin, line)) {
    // 1. Check main query match / invert match (-U)
    bool matches = matches_query(line, query);
    if (invert_match) {
      if (matches && !query.empty())
        continue;
    } else {
      if (!matches)
        continue;
    }

    // 2. Check disinclude query (-D)
    if (has_disinclude && matches_query(line, disinclude_query)) {
      continue;
    }

    // 3. String replacement (-T TEMPL REPL)
    std::string processed_line = line;
    if (has_replace) {
      size_t pos = 0;
      while ((pos = processed_line.find(templ, pos)) != std::string::npos) {
        processed_line.replace(pos, templ.length(), repl);
        pos += repl.length();
      }
    }

    // 4. URL highlighting (-u)
    if (highlight_url_flag) {
      processed_line = highlight_urls(processed_line);
    }

    // Accumulate stats and output
    accumulated_output << processed_line << "\n";
    line_count++;
    char_count += processed_line.length() + 1; // +1 for newline

    // Count words
    std::stringstream ss(processed_line);
    std::string word;
    while (ss >> word) {
      word_count++;
    }
  }

  std::string final_output = accumulated_output.str();

  // Append `wc` stats if requested (-E)
  if (count_stats) {
    std::stringstream stats;
    stats << "  " << line_count << "  " << word_count << "  " << char_count
          << "\n";
    final_output += stats.str();
  }

  // Output routing
  if (to_stdout || output_file.empty()) {
    std::cout << final_output;
  }

  if (!to_stdout && !output_file.empty()) {
    std::ofstream outfile(output_file);
    if (outfile.is_open()) {
      outfile << final_output;
      outfile.close();
    } else {
      std::cerr << "Error: Could not open file " << output_file
                << " for writing.\n";
    }
  }

  // Default clipboard copy if neither -S nor -F overrides clipboard destination
  if (!to_stdout && output_file.empty()) {
    if (!write_to_clipboard(final_output)) {
      std::cerr << "Warning: Failed to copy to system clipboard. Falling back "
                   "to stdout.\n";
      std::cout << final_output;
    }
  }

  return 0;
}
