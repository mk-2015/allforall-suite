#include <iostream>
#include <chrono>
#include <thread>
#include <string_view>
#include <string> 

void _sleep(double milliseconds) {
    auto start = std::chrono::high_resolution_clock::now();
    
    if (milliseconds > 2.0) {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(milliseconds - 2.0));
    }
    
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - start).count();
        
        if (elapsed >= milliseconds) {
            break; 
        }
        
        if (milliseconds - elapsed > 0.1) {
            std::this_thread::yield();
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 0) {
        std::string_view prog_name = argv[0];
        if (prog_name.find("nothing-copyright") != std::string_view::npos) {
            std::cout << "mk-2015 (C) allforall tools, Nothing tool\n";
        }
    }
    
    double time = -1.0;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            std::cout << "-h or --help  := help menu\n";
            std::cout << "-s or --sleep := sleep in milliseconds\n";
        } else if (arg == "-s" || arg == "--sleep") {
            if (i + 1 < argc) { 
                try {
                    time = std::stod(argv[++i]); 
                } catch (...) {
                    std::cerr << "Error: Invalid number format for sleep duration.\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: " << arg << " requires millisecond parameter.\n";
                return 1;
            }
        }
    }
    
    if (time >= 0.0) {
        _sleep(time);
    }
    
    return 0;
}