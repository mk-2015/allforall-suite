#include <iostream>
#include <string>
#include <qu/qu.hpp>

#if defined(_WIN32)
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace que {

std::string Password(const std::string& prompt, bool star) {
    std::cout << "· " << prompt << "—> ";
    std::cout.flush();

    std::string password;

#if defined(_WIN32)
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);

    if (star) {
        char c = 0;
        DWORD read = 0;
        while (ReadFile(hStdin, &c, 1, &read, NULL) && read > 0) {
            if (c == '\r' || c == '\n') {
                std::cout << "\n";
                break;
            }
            if (c == '\b' || c == 127) {
                if (!password.empty()) {
                    password.pop_back();
                    std::cout << "\b \b";
                    std::cout.flush();
                }
            } else {
                password.push_back(c);
                std::cout << '*';
                std::cout.flush();
            }
        }
    } else {
        std::getline(std::cin, password);
        std::cout << "\n";
    }

    SetConsoleMode(hStdin, mode);
#else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    if (star) {
        newt.c_lflag &= ~ICANON;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if (star) {
        char c = 0;
        while (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == '\n' || c == '\r') {
                std::cout << "\n";
                break;
            }
            if (c == 127 || c == '\b') {
                if (!password.empty()) {
                    password.pop_back();
                    std::cout << "\b \b";
                    std::cout.flush();
                }
            } else {
                password.push_back(c);
                std::cout << '*';
                std::cout.flush();
            }
        }
    } else {
        std::getline(std::cin, password);
        std::cout << "\n";
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    return password;
}

} // namespace que
