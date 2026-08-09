#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <pkg_db.hpp>

namespace fs = std::filesystem;

fs::path get_keyring_dir() {
    fs::path home_dir;

#if defined(_WIN32) || defined(_WIN64)
    const char* user_profile = std::getenv("USERPROFILE");
    if (user_profile) {
        home_dir = user_profile;
    } else {
        const char* home_drive = std::getenv("HOMEDRIVE");
        const char* home_path = std::getenv("HOMEPATH");
        if (home_drive && home_path) {
            home_dir = std::string(home_drive) + std::string(home_path);
        } else {
            throw std::runtime_error("Critical Error: Windows user home directory environment variables not found.");
        }
    }
#else
    const char* home = std::getenv("HOME");
    if (!home) {
        throw std::runtime_error("Critical Error: HOME environment variable not set.");
    }
    home_dir = home;
#endif

    return home_dir / ".hpm" / "keyring";
}

bool verify_with_maintainer_key(const std::string& maintainer_name, 
                                const std::vector<unsigned char>& file_data, 
                                const std::vector<unsigned char>& signature) {
    
    fs::path pub_key_path = get_keyring_dir() / (maintainer_name + ".sig.pub");
    
    if (!fs::exists(pub_key_path)) {
        std::cerr << "Error: Public key for maintainer '" << maintainer_name << "' not found at " << pub_key_path << "\n";
        return false;
    }

    bool success = false;
    BIO* bio = BIO_new_file(pub_key_path.string().c_str(), "r");
    if (!bio) return false;

    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (!pkey) return false;

    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    if (md_ctx) {
        if (EVP_DigestVerifyInit(md_ctx, NULL, NULL, NULL, pkey) == 1) {
            int ret = EVP_DigestVerify(
                md_ctx, 
                signature.data(), signature.size(), 
                file_data.data(), file_data.size()
            );
            if (ret == 1) {
                success = true;
            }
        }
        EVP_MD_CTX_free(md_ctx);
    }

    EVP_PKEY_free(pkey);
    return success;
}

// 1. Add a maintainer's public key by copying/importing it into ~/.hpm/keyring/
bool keyring_add_maintainer(std::string name, fs::path keyfile) {
    if (!fs::exists(keyfile)) {
        std::cerr << "Error: Key file does not exist at " << keyfile << "\n";
        return false;
    }

    fs::path keyring_dir = get_keyring_dir();
    fs::create_directories(keyring_dir);

    fs::path destination = keyring_dir / (name + ".sig.pub");

    try {
        // Validate that the file is actually a valid PEM public key before saving
        BIO* bio = BIO_new_file(keyfile.string().c_str(), "r");
        if (!bio) {
            std::cerr << "Error: Could not open key file.\n";
            return false;
        }
        EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
        BIO_free(bio);

        if (!pkey) {
            std::cerr << "Error: Provided file is not a valid PEM public key.\n";
            return false;
        }
        EVP_PKEY_free(pkey);

        // Copy valid key to the keyring directory
        fs::copy_file(keyfile, destination, fs::copy_options::overwrite_existing);
        std::cout << "===> Successfully added public key for '" << name << "' to keyring.\n";
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
        return false;
    }
}

// 2. Remove a maintainer from the keyring
bool keyring_remove_maintainer(std::string name) {
    fs::path keyring_dir = get_keyring_dir();
    fs::path target_key = keyring_dir / (name + ".sig.pub");

    if (!fs::exists(target_key)) {
        std::cerr << "Error: Maintainer '" << name << "' not found in keyring.\n";
        return false;
    }

    try {
        fs::remove(target_key);
        std::cout << "===> Removed maintainer '" << name << "' from keyring.\n";
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
        return false;
    }
}

// 3. List all installed maintainer public keys
std::vector<std::string> keyring_list_maintainers() {
    std::vector<std::string> maintainers;
    fs::path keyring_dir = get_keyring_dir();

    if (!fs::exists(keyring_dir)) {
        return maintainers;
    }

    for (const auto& entry : fs::directory_iterator(keyring_dir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            // Look for files ending with ".sig.pub"
            if (filename.length() > 8 && filename.substr(filename.length() - 8) == ".sig.pub") {
                std::string name = filename.substr(0, filename.length() - 8);
                maintainers.push_back(name);
            }
        }
    }

    return maintainers;
}

// 4. Generate a new Ed25519 keypair (Private + Public) for package maintainers
bool keyring_generate_maintainer(std::string name, std::string email, fs::path private_keyring_path) {
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, NULL);

    if (!pctx) {
        std::cerr << "Error: Ed25519 algorithm not supported by this OpenSSL version.\n";
        return false;
    }

    if (EVP_PKEY_keygen_init(pctx) <= 0 || EVP_PKEY_keygen(pctx, &pkey) <= 0) {
        std::cerr << "Error: Failed to generate Ed25519 key pair.\n";
        EVP_PKEY_CTX_free(pctx);
        return false;
    }
    EVP_PKEY_CTX_free(pctx);

    // Save Private Key (PEM format)
    FILE* priv_file = fopen(private_keyring_path.string().c_str(), "wb");
    if (!priv_file) {
        std::cerr << "Error: Could not open path for writing private key: " << private_keyring_path << "\n";
        EVP_PKEY_free(pkey);
        return false;
    }
    PEM_write_PrivateKey(priv_file, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(priv_file);

    fs::path keyring_dir = get_keyring_dir();
    fs::create_directories(keyring_dir);
    fs::path pub_key_destination = keyring_dir / (name + ".sig.pub");

    fs::path email_meta_path = keyring_dir / (name + ".email");
    std::ofstream email_file(email_meta_path);
    if (email_file.is_open()) {
        email_file << email;
        email_file.close();
    }

    FILE* pub_file = fopen(pub_key_destination.string().c_str(), "wb");
    if (!pub_file) {
        std::cerr << "Error: Could not write public key to keyring directory.\n";
        EVP_PKEY_free(pkey);
        return false;
    }
    PEM_write_PUBKEY(pub_file, pkey);
    fclose(pub_file);

    EVP_PKEY_free(pkey);

    std::cout << "===> Generated new Ed25519 keys successfully with comment/email: " << email << "\n";
    std::cout << "     Private Key saved to: " << private_keyring_path << "\n";
    std::cout << "     Public Key registered for '" << name << "' at: " << pub_key_destination << "\n";
    return true;
}

bool keyring_extract_maintainer(std::string name, fs::path output_path) {
    fs::path keyring_dir = get_keyring_dir();
    fs::path target_key = keyring_dir / (name + ".sig.pub");

    if (!fs::exists(target_key)) {
        std::cerr << "Error: Maintainer '" << name << "' not found in keyring at " << target_key << "\n";
        return false;
    }

    try {
        fs::copy_file(target_key, output_path, fs::copy_options::overwrite_existing);
        std::cout << "===> Successfully extracted public key for '" << name << "' to " << output_path << "\n";
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error during key extraction: " << e.what() << "\n";
        return false;
    }
}   

bool keyring_sign_archive(fs::path private_key_path, fs::path target_file_path, fs::path output_sig_path) {
    if (!fs::exists(private_key_path)) {
        std::cerr << "Error: Private key not found at " << private_key_path << "\n";
        return false;
    }
    if (!fs::exists(target_file_path)) {
        std::cerr << "Error: Target file to sign not found at " << target_file_path << "\n";
        return false;
    }

    // 1. Load Private Key from PEM
    BIO* bio = BIO_new_file(private_key_path.string().c_str(), "r");
    if (!bio) {
        std::cerr << "Error: Could not open private key file.\n";
        return false;
    }

    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);

    if (!pkey) {
        std::cerr << "Error: Failed to parse private key (invalid PEM format).\n";
        return false;
    }

    // 2. Read target file bytes into memory
    std::vector<unsigned char> file_data;
    {
        std::ifstream file(target_file_path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open target file for reading.\n";
            EVP_PKEY_free(pkey);
            return false;
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        file_data.resize(size);
        if (!file.read((char*)file_data.data(), size)) {
            std::cerr << "Error: Failed to read target file bytes.\n";
            EVP_PKEY_free(pkey);
            return false;
        }
    }

    // 3. Initialize Signature Context
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) {
        std::cerr << "Error: Failed to create OpenSSL digest context.\n";
        EVP_PKEY_free(pkey);
        return false;
    }

    bool success = false;
    std::vector<unsigned char> signature;

    if (EVP_DigestSignInit(md_ctx, NULL, NULL, NULL, pkey) == 1) {
        size_t sig_len = 0;
        
        // First call to get required signature length
        if (EVP_DigestSign(md_ctx, NULL, &sig_len, file_data.data(), file_data.size()) == 1) {
            signature.resize(sig_len);
            
            // Second call to actually generate the signature
            if (EVP_DigestSign(md_ctx, signature.data(), &sig_len, file_data.data(), file_data.size()) == 1) {
                signature.resize(sig_len);
                success = true;
            }
        }
    }

    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(pkey);

    if (!success) {
        std::cerr << "Error: Ed25519 signing operation failed.\n";
        return false;
    }

    std::ofstream sig_file(output_sig_path, std::ios::binary);
    if (!sig_file.is_open()) {
        std::cerr << "Error: Could not write signature output to " << output_sig_path << "\n";
        return false;
    }
    sig_file.write((char*)signature.data(), signature.size());
    sig_file.close();

    std::cout << "===> Successfully signed '" << target_file_path.filename().string() 
              << "' -> Saved signature to " << output_sig_path << "\n";
    return true;
}