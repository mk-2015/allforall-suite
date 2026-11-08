#include <iostream>
#include <algorithm>
#include <cctype>
#include <qu/qu.hpp>

namespace que {

std::string Ask(const std::string& prompt) {
    std::cout << "· " << prompt << "—> ";
    std::cout.flush();

    std::string input;
    std::getline(std::cin, input);
    return input;
}

bool Confirm(const std::string& prompt, bool default_yes) {
    std::string suffix = default_yes ? " [Y/n]" : " [y/N]";
    
    while (true) {
        std::string response = Ask(prompt + suffix);
        if (response.empty()) {
            return default_yes;
        }

        std::string lower = response;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (lower == "y" || lower == "yes" || lower == "true" || lower == "1") {
            return true;
        }
        if (lower == "n" || lower == "no" || lower == "false" || lower == "0") {
            return false;
        }

        std::cout << "Please answer yes or no.\n";
    }
}

int Select(const std::string& prompt, const std::vector<std::string>& choices) {
    if (!prompt.empty()) {
        std::cout << prompt << "\n";
    }
    for (size_t i = 0; i < choices.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << choices[i] << "\n";
    }

    while (true) {
        std::string response = Ask("Choice (1-" + std::to_string(choices.size()) + ")");
        if (response.empty()) {
            continue;
        }

        try {
            int idx = std::stoi(response);
            if (idx >= 1 && static_cast<size_t>(idx) <= choices.size()) {
                return idx - 1;
            }
        } catch (...) {
        }

        std::cout << "Invalid selection. Please enter a number between 1 and " << choices.size() << ".\n";
    }
}

} // namespace que
