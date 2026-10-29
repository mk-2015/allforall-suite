#include <iostream>
#include <stdexcept>
#include <new>
#include <string>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #include <bcrypt.h>

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
    #include <stdlib.h>
#elif defined(__linux__)
    #include <sys/random.h>
    #include <unistd.h>
    #include <cerrno>
#else
    #error "Unsupported platform for native CSPRNG API"
#endif

char* getrng(unsigned int bytes) {
    if (bytes == 0) return NULL;

    char* buffer = new char[bytes];

#if defined(_WIN32) || defined(_WIN64)
    NTSTATUS status = BCryptGenRandom(
        NULL,
        reinterpret_cast<PUCHAR>(buffer),
        static_cast<ULONG>(bytes),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );

    if (status != 0) {
        delete[] buffer;
        return NULL;
    }

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
    arc4random_buf(buffer, static_cast<size_t>(bytes));

#elif defined(__linux__)
    unsigned int total_read = 0;
    while (total_read < bytes) {
        ssize_t result = getrandom(
            buffer + total_read,
            bytes - total_read,
            0
        );

        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            delete[] buffer;
            return NULL;
        }
        total_read += static_cast<unsigned int>(result);
    }
#endif

    return buffer;
}

void print_help() {
    std::cout << "sysrng [OPTIONS] [BYTES]\n\n"
              << "Options:\n"
              << "    -h      : output in hex, can be used with -f\n"
              << "    -b      : output in binary, can be used with -f\n"
              << "    -o      : output in octal, can be used with -f\n"
              << "    -f FILE : Output data into FILE\n"
              << "    --help  : Help menu\n"
              << "    -r      : Output raw bytes, can only be used with -f\n";
}

int main(int argc, char* argv[]) {
    bool hex_out = false;
    bool bin_out = false;
    bool oct_out = false;
    bool raw_out = false;
    std::string filename = "";
    unsigned int bytes = 32;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h") {
            if (argc == 2) {
                print_help();
                return 0;
            }
            hex_out = true;
        } else if (arg == "--help" || arg == "-?") {
            print_help();
            return 0;
        } else if (arg == "-b") {
            bin_out = true;
        } else if (arg == "-o") {
            oct_out = true;
        } else if (arg == "-r") {
            raw_out = true;
        } else if (arg == "-f") {
            if (i + 1 < argc) {
                filename = argv[++i];
            } else {
                std::cerr << "Error: -f requires a filename argument.\n";
                return 1;
            }
        } else {
            try {
                long val = std::stoul(arg);
                if (val <= 0) {
                    std::cerr << "Error: BYTES must be a positive integer.\n";
                    return 1;
                }
                bytes = static_cast<unsigned int>(val);
            } catch (...) {
                std::cerr << "Error: Invalid argument or BYTES value: " << arg << "\n";
                return 1;
            }
        }
    }

    if (!hex_out && !bin_out && !oct_out && !raw_out) {
        if (filename.empty()) {
            hex_out = true;
        } else {
            raw_out = true;
        }
    }

    if (raw_out && filename.empty()) {
        std::cerr << "Error: -r (raw bytes) can only be used with -f.\n";
        return 1;
    }

    int format_count = (hex_out ? 1 : 0) + (bin_out ? 1 : 0) + (oct_out ? 1 : 0) + (raw_out ? 1 : 0);
    if (format_count > 1) {
        std::cerr << "Error: Conflicting output format options specified.\n";
        return 1;
    }

    char* buffer = getrng(bytes);
    if (!buffer) {
        std::cerr << "Error: Failed to generate random bytes from CSPRNG.\n";
        return 1;
    }

    std::string output_data = "";
    if (raw_out) {
        output_data.assign(buffer, bytes);
    } else if (hex_out) {
        for (unsigned int i = 0; i < bytes; ++i) {
            char hex_buf[3];
            std::snprintf(hex_buf, sizeof(hex_buf), "%02x", static_cast<unsigned char>(buffer[i]));
            output_data.append(hex_buf);
        }
        output_data.append("\n");
    } else if (bin_out) {
        for (unsigned int i = 0; i < bytes; ++i) {
            unsigned char b = static_cast<unsigned char>(buffer[i]);
            for (int bit = 7; bit >= 0; --bit) {
                output_data.push_back(((b >> bit) & 1) ? '1' : '0');
            }
        }
        output_data.append("\n");
    } else if (oct_out) {
        for (unsigned int i = 0; i < bytes; ++i) {
            char oct_buf[4];
            std::snprintf(oct_buf, sizeof(oct_buf), "%03o", static_cast<unsigned char>(buffer[i]));
            output_data.append(oct_buf);
        }
        output_data.append("\n");
    }

    delete[] buffer;

    if (!filename.empty()) {
        std::ofstream outfile(filename, std::ios::binary);
        if (!outfile) {
            std::cerr << "Error: Failed to open file for writing: " << filename << "\n";
            return 1;
        }
        outfile.write(output_data.data(), output_data.size());
        outfile.close();
    } else {
        std::cout << output_data;
    }

    return 0;
}
