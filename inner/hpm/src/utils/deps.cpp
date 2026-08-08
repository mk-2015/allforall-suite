#include <deps.hpp>
#include <pkg.hpp>

#include <curl/curl.h>
#include <miniz.h>

#include <cstdio>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstring>
#include <vector>

namespace fs = std::filesystem;

static size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    return fwrite(ptr, size, nmemb, stream);
}

bool download_file(const std::string& url, const std::string& output_path) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    fclose(fp);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && response_code == 200);
}

bool extract_zip(const std::string& archive_path, const std::string& target_dir) {
    mz_zip_archive zip_archive;
    std::memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_reader_init_file(&zip_archive, archive_path.c_str(), 0)) {
        hpm_fail((int)NULL, "Failed to initialize zip reader for file: %s\n", archive_path.c_str());
        return false;
    }

    mz_uint num_files = mz_zip_reader_get_num_files(&zip_archive);

    for (mz_uint i = 0; i < num_files; ++i) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
            continue;
        }

        fs::path dest_path = fs::path(target_dir) / file_stat.m_filename;

        if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
            fs::create_directories(dest_path);
        } else {
            if (dest_path.has_parent_path()) {
                fs::create_directories(dest_path.parent_path());
            }

            if (!mz_zip_reader_extract_to_file(&zip_archive, i, dest_path.string().c_str(), 0)) {
                hpm_fail((int)NULL, "Failed to extract file: %s\n", file_stat.m_filename);
                mz_zip_reader_end(&zip_archive);
                return false;
            }
        }
    }

    mz_zip_reader_end(&zip_archive);
    return true;
}