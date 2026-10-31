#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <openssl/evp.h>
#include <openssl/kdf.h>

namespace fs = std::filesystem;

struct Options {
    std::string filename;
    std::string salt_filename;
    std::string pepper_filename;
    std::string algorithm = "sha256";
    bool show_help = false;
};

/*
 @param: const std::string& input, String to normalize
 @desc: Converts input string to lower case for comparison
*/
std::string to_lower(std::string input) {
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return input;
}

/*
 @param: const std::vector<unsigned char>& data, Binary data buffer
 @desc: Converts binary data buffer to a hexadecimal string
*/
std::string bytes_to_hex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    for (unsigned char byte : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

/*
 @param: const std::string& path, Target file path
 @param: std::vector<unsigned char>& buffer, Destination byte vector
 @desc: Reads an entire file into a binary byte vector
*/
bool read_file_bytes(const std::string& path, std::vector<unsigned char>& buffer) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    buffer.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return true;
}

/*
 @param: const std::string& path, Target file path
 @param: const std::vector<unsigned char>& salt, Salt data buffer
 @param: const std::vector<unsigned char>& pepper, Pepper data buffer
 @param: const EVP_MD* md_type, OpenSSL digest function pointer
 @param: std::string& hex_output, Reference string for output
 @desc: Calculates cryptographic hash for target file appended with salt and pepper
*/
bool compute_evp_hash(const std::string& path, const std::vector<unsigned char>& salt, const std::vector<unsigned char>& pepper, const EVP_MD* md_type, std::string& hex_output) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        return false;
    }

    if (EVP_DigestInit_ex(ctx, md_type, nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return false;
    }

    constexpr std::size_t buffer_size = 65536;
    std::vector<char> buffer(buffer_size);

    while (file.read(buffer.data(), buffer_size) || file.gcount() > 0) {
        if (EVP_DigestUpdate(ctx, buffer.data(), file.gcount()) != 1) {
            EVP_MD_CTX_free(ctx);
            return false;
        }
    }

    if (!salt.empty()) {
        if (EVP_DigestUpdate(ctx, salt.data(), salt.size()) != 1) {
            EVP_MD_CTX_free(ctx);
            return false;
        }
    }

    if (!pepper.empty()) {
        if (EVP_DigestUpdate(ctx, pepper.data(), pepper.size()) != 1) {
            EVP_MD_CTX_free(ctx);
            return false;
        }
    }

    unsigned int len = 0;
    std::vector<unsigned char> hash(EVP_MAX_MD_SIZE);
    if (EVP_DigestFinal_ex(ctx, hash.data(), &len) != 1) {
        EVP_MD_CTX_free(ctx);
        return false;
    }

    EVP_MD_CTX_free(ctx);
    hash.resize(len);
    hex_output = bytes_to_hex(hash);
    return true;
}

/*
 @param: const std::string& path, Target file path
 @param: const std::vector<unsigned char>& salt, Salt data buffer
 @param: const std::vector<unsigned char>& pepper, Pepper data buffer
 @param: std::string& hex_output, Reference string for output
 @desc: Computes Argon2id hash using file data, salt, and optional pepper
*/
bool compute_argon2_hash(const std::string& path, const std::vector<unsigned char>& salt, const std::vector<unsigned char>& pepper, std::string& hex_output) {
    std::vector<unsigned char> content;
    if (!read_file_bytes(path, content)) {
        return false;
    }

    if (!pepper.empty()) {
        content.insert(content.end(), pepper.begin(), pepper.end());
    }

    EVP_KDF* kdf = EVP_KDF_fetch(nullptr, "ARGON2ID", nullptr);
    if (!kdf) {
        return false;
    }

    EVP_KDF_CTX* kctx = EVP_KDF_CTX_new(kdf);
    EVP_KDF_free(kdf);
    if (!kctx) {
        return false;
    }

    std::vector<unsigned char> effective_salt = salt;
    if (effective_salt.empty()) {
        std::string fallback_salt = "static_hashbin_salt";
        effective_salt.assign(fallback_salt.begin(), fallback_salt.end());
    }

    uint32_t threads = 1;
    uint32_t memory = 4096;
    uint32_t time = 3;

    OSSL_PARAM params[] = {
        OSSL_PARAM_construct_octet_string("pass", content.data(), content.size()),
        OSSL_PARAM_construct_octet_string("salt", effective_salt.data(), effective_salt.size()),
        OSSL_PARAM_construct_uint32("iter", &time),
        OSSL_PARAM_construct_uint32("mem", &memory),
        OSSL_PARAM_construct_uint32("threads", &threads),
        OSSL_PARAM_construct_end()
    };

    std::vector<unsigned char> out(32);
    bool success = (EVP_KDF_derive(kctx, out.data(), out.size(), params) > 0);
    EVP_KDF_CTX_free(kctx);

    if (success) {
        hex_output = bytes_to_hex(out);
    }
    return success;
}

/*
 @desc: Displays CLI options and supported hash algorithms
*/
void print_help() {
    std::cout << "hashbin - File Hashing Utility\n\n"
              << "USAGE:\n"
              << "    hashbin <filename> [OPTIONS]\n\n"
              << "OPTIONS:\n"
              << "    -a, --algorithm <algo>   Select hashing algorithm (default: sha256)\n"
              << "    -s <saltfile>            File containing salt bytes\n"
              << "    -p <pepperfile>          File containing pepper bytes\n"
              << "    -h, --help               Display help menu\n\n"
              << "SUPPORTED ALGORITHMS:\n"
              << "    sha256, sha512, sha1, md5, argon2\n";
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @param: Options& opts, Target Options structure
 @desc: Parses CLI flags and parameters into Options structure
*/
bool parse_args(int argc, char* argv[], Options& opts) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.show_help = true;
            return true;
        } else if (arg == "-a" || arg == "--algorithm" || arg == "--algorithim") {
            if (i + 1 < argc) {
                opts.algorithm = argv[++i];
            } else {
                return false;
            }
        } else if (arg == "-s") {
            if (i + 1 < argc) {
                opts.salt_filename = argv[++i];
            } else {
                return false;
            }
        } else if (arg == "-p") {
            if (i + 1 < argc) {
                opts.pepper_filename = argv[++i];
            } else {
                return false;
            }
        } else if (opts.filename.empty()) {
            opts.filename = arg;
        } else {
            return false;
        }
    }
    return !opts.filename.empty() || opts.show_help;
}

/*
 @param: int argc, Argument count
 @param: char* argv[], Argument values
 @desc: Main execution entry point for hashbin tool
*/
int main(int argc, char* argv[]) {
    Options opts;
    if (!parse_args(argc, argv, opts)) {
        print_help();
        return 1;
    }

    if (opts.show_help) {
        print_help();
        return 0;
    }

    if (!fs::exists(opts.filename)) {
        std::cerr << "Error: Target file not found.\n";
        return 1;
    }

    std::vector<unsigned char> salt;
    if (!opts.salt_filename.empty()) {
        if (!fs::exists(opts.salt_filename) || !read_file_bytes(opts.salt_filename, salt)) {
            std::cerr << "Error: Unable to read salt file.\n";
            return 1;
        }
    }

    std::vector<unsigned char> pepper;
    if (!opts.pepper_filename.empty()) {
        if (!fs::exists(opts.pepper_filename) || !read_file_bytes(opts.pepper_filename, pepper)) {
            std::cerr << "Error: Unable to read pepper file.\n";
            return 1;
        }
    }

    std::string algo = to_lower(opts.algorithm);
    std::string result_hash;
    std::string display_algo = opts.algorithm;
    bool success = false;

    if (algo == "sha256") {
        display_algo = "SHA256";
        success = compute_evp_hash(opts.filename, salt, pepper, EVP_sha256(), result_hash);
    } else if (algo == "sha512") {
        display_algo = "SHA512";
        success = compute_evp_hash(opts.filename, salt, pepper, EVP_sha512(), result_hash);
    } else if (algo == "sha1") {
        display_algo = "SHA1";
        success = compute_evp_hash(opts.filename, salt, pepper, EVP_sha1(), result_hash);
    } else if (algo == "md5") {
        display_algo = "MD5";
        success = compute_evp_hash(opts.filename, salt, pepper, EVP_md5(), result_hash);
    } else if (algo == "argon2") {
        display_algo = "ARGON2";
        success = compute_argon2_hash(opts.filename, salt, pepper, result_hash);
    } else {
        std::cerr << "Error: Unsupported algorithm '" << opts.algorithm << "'\n";
        return 1;
    }

    if (!success) {
        std::cerr << "Error: Failed to compute hash.\n";
        return 1;
    }

    std::cout << display_algo << "(" << opts.filename << "," 
              << opts.salt_filename << "," 
              << opts.pepper_filename << ")=" 
              << result_hash << "\n";

    return 0;
}