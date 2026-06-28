#include <httplib.h>
#include <cmp.hpp>
#include <pkg.hpp>
#include <vector>
#include <print>
#include <fstream>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int DownloadPackageArchive(const std::string& hostUrl, const std::string& path, const fs::path& targetDestination)
{
    httplib::Client cli(hostUrl);
    cli.set_follow_location(true);

    fs::path temp = targetDestination;
    temp += ".part";

    std::ofstream outFile(temp, std::ios::binary);
    if (!outFile.is_open())
    {
        std::println("[SNIPE] Error: Failed to open target file for writing.");
        return -1;
    }

    std::println("[SNIPE] Streaming download from {}{}...", hostUrl, path);

    bool write_error = false;
    auto res = cli.Get(path.c_str(),
        [&](const char *data, size_t data_length)
        {
            if (write_error)
            {
                return false;
            }

            outFile.write(data, static_cast<std::streamsize>(data_length));
            if (!outFile)
            {
                write_error = true;
                return false;
            }

            return true;
        }
    );

    outFile.close();

    if (!res)
    {
        std::println("[SNIPE] Network Error: {}", to_string(res.error()));
        if (fs::exists(temp))
        {
            fs::remove(temp);
        }
        return -2;
    }

    if (res->status != 200)
    {
        std::println("[SNIPE] HTTP Download Failed. Status: {}", res->status);
        if (fs::exists(temp))
        {
            fs::remove(temp);
        }
        return -3;
    }

    if (write_error)
    {
        std::println("[SNIPE] Download aborted due to local write error.");
        if (fs::exists(temp))
        {
            fs::remove(temp);
        }
        return -4;
    }

    std::error_code ec;
    fs::rename(temp, targetDestination, ec);
    if (ec)
    {
        std::println("[SNIPE] Failed to move file into place: {}", ec.message());
        if (fs::exists(temp))
        {
            fs::remove(temp);
        }
        return -5;
    }

    std::println("[SNIPE] Archive successfully downloaded to: {}", targetDestination.string());
    return 0;
}

int DownloadPackage(std::string PackageName, std::string PackageVersion)
{
    std::vector<std::string> repo;
    fs::path packrepoFile = GetSnipeInstallDir() / "packrepo.lst";

    std::ifstream infile(packrepoFile);
    if (!infile.is_open())
    {
        std::println("[SNIPE] Cannot open repository file.");
        return -1;
    }

    std::string currentLine;
    while (std::getline(infile, currentLine))
    {
        if (currentLine.empty() || currentLine.starts_with('#'))
        {
            continue;
        }
        repo.push_back(currentLine);
    }

    if (repo.empty())
    {
        std::println("[SNIPE] No repositories configured.");
        return -2;
    }

    fs::path destDir = GetSnipeInstallDir() / "cache";
    fs::create_directories(destDir);

    fs::path archivePath = destDir / (PackageName + "-" + PackageVersion + ".tar.gz");

    for (const auto& r : repo)
    {
        std::string host = r;
        std::string basePath = "/";

        if (host.rfind("https://", 0) == 0)
        {
            host = host.substr(8);
        }
        else if (host.rfind("http://", 0) == 0)
        {
            host = host.substr(7);
        }

        auto slashPos = host.find('/');
        if (slashPos != std::string::npos)
        {
            basePath = host.substr(slashPos);
            host = host.substr(0, slashPos);
        }

        if (basePath.empty())
        {
            basePath = "/";
        }

        if (!basePath.ends_with('/'))
        {
            basePath.push_back('/');
        }

        std::string packagePath = basePath + PackageName + "/" + PackageVersion + "/" + PackageName + "-" + PackageVersion + ".tar.gz";

        int rc = DownloadPackageArchive(host, packagePath, archivePath);
        if (rc == 0)
        {
            std::println("[SNIPE] Download complete.");
            return 0;
        }

        int rc2 = UntargzArchive(archivePath, GetSnipeInstallDir() / "packages" / "");

        std::println("[SNIPE] Repository {} failed with code {}", r, rc);
    }

    std::println("[SNIPE] All repositories failed.");
    return -3;
}

int InstallPackage(std::string PackageName, std::string PackageVersion)
{
    int DownloadPackageReturnValue = DownloadPackage(PackageName, PackageVersion);
}