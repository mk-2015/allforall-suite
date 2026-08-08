#include <pkg.hpp>
#include <string>
#include <iostream>
#include <cstdarg>
#include <cstdlib>

static void hpm_log_vprint(const char* prefix, const std::string& str, std::va_list args) {
    std::cout << prefix << " ";
    
    vfprintf(stdout, str.c_str(), args);
    std::cout << std::endl;
}

void hpm_ok(std::string str, ...) {
    std::va_list args;
    va_start(args, str);
    hpm_log_vprint("[   OK   ]  ", str, args);
    va_end(args);
}

void hpm_fail(int autoexit, std::string str, ...) {
    std::va_list args;
    va_start(args, str);
    
    std::cerr << "[  FAIL  ] ";
    vfprintf(stderr, str.c_str(), args);
    std::cerr << std::endl;
    
    va_end(args);

    if (autoexit) {
        std::exit(autoexit);
    }
}

void hpm_info(std::string str, ...) {
    std::va_list args;
    va_start(args, str);
    hpm_log_vprint("[  INFO  ]", str, args);
    va_end(args);
}