#include <iostream>
#include <string>
#include <optional>
#include <qu/qu.hpp>

namespace que {

int Int(const std::string& prompt, std::optional<int> min_val, std::optional<int> max_val) {
    while (true) {
        std::string response = Ask(prompt);
        if (response.empty()) {
            continue;
        }

        try {
            size_t idx = 0;
            int val = std::stoi(response, &idx);
            if (idx == response.size()) {
                if (min_val && val < *min_val) {
                    std::cout << "Value must be at least " << *min_val << ".\n";
                    continue;
                }
                if (max_val && val > *max_val) {
                    std::cout << "Value must be at most " << *max_val << ".\n";
                    continue;
                }
                return val;
            }
        } catch (...) {
        }

        std::cout << "Invalid integer. Please try again.\n";
    }
}

double Float(const std::string& prompt, std::optional<double> min_val, std::optional<double> max_val) {
    while (true) {
        std::string response = Ask(prompt);
        if (response.empty()) {
            continue;
        }

        try {
            size_t idx = 0;
            double val = std::stod(response, &idx);
            if (idx == response.size()) {
                if (min_val && val < *min_val) {
                    std::cout << "Value must be at least " << *min_val << ".\n";
                    continue;
                }
                if (max_val && val > *max_val) {
                    std::cout << "Value must be at most " << *max_val << ".\n";
                    continue;
                }
                return val;
            }
        } catch (...) {
        }

        std::cout << "Invalid number. Please try again.\n";
    }
}

} // namespace que
