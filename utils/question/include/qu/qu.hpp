#pragma once

#include <string>
#include <vector>
#include <optional>

namespace que {

// ANSI Colors
constexpr const char* COLOR_BLACK   = "\033[30m";
constexpr const char* COLOR_RED     = "\033[31m";
constexpr const char* COLOR_GREEN   = "\033[32m";
constexpr const char* COLOR_YELLOW  = "\033[33m";
constexpr const char* COLOR_BLUE    = "\033[34m";
constexpr const char* COLOR_MAGENTA = "\033[35m";
constexpr const char* COLOR_CYAN    = "\033[36m";
constexpr const char* COLOR_WHITE   = "\033[37m";

// ANSI Styles
constexpr const char* STYLE_BOLD      = "\033[1m";
constexpr const char* STYLE_ITALIC    = "\033[3m";
constexpr const char* STYLE_UNDERLINE = "\033[4m";
constexpr const char* STYLE_DIM       = "\033[2m";
constexpr const char* RESET           = "\033[0m";

// Ask prompts the user with the given prompt and returns the entered string.
// Prints '· <prompt>—> ' when getting input.
std::string Ask(const std::string& prompt);

// Confirm prompts the user for a confirmation (Yes/No).
bool Confirm(const std::string& prompt, bool default_yes = true);

// Select prompts the user to choose an option from a list of choices.
int Select(const std::string& prompt, const std::vector<std::string>& choices);

// Password prompts the user for a password with input masking.
std::string Password(const std::string& prompt, bool star = false);

// Int prompts the user for an integer within an optional range.
int Int(const std::string& prompt, std::optional<int> min_val = std::nullopt, std::optional<int> max_val = std::nullopt);

// Float prompts the user for a floating-point number within an optional range.
double Float(const std::string& prompt, std::optional<double> min_val = std::nullopt, std::optional<double> max_val = std::nullopt);

} // namespace que
