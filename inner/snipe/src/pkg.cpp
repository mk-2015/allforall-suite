#include <httplib.h>
#include <cmp.hpp>
#include <pkg.hpp>
#include <vector>
#include <print>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace {

std::vector<std::string> gInstallingPackages;

std::string TrimWhitespace(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string ToLowerCopy(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool IsLatestVersionRequest(const std::string& version)
{
    return ToLowerCopy(version) == "latest";
}

bool ResolveLatestVersion(const std::string& hostUrl, const std::string& basePath, const std::string& packageName, std::string& resolvedVersion)
{
    std::string latestPath = basePath + packageName + "/.latest";
    httplib::Client cli(hostUrl);
    cli.set_follow_location(true);

    auto res = cli.Get(latestPath.c_str());
    if (!res)
    {
        std::println("[SNIPE] Failed to read {}{} from {}", hostUrl, latestPath, packageName);
        return false;
    }

    if (res->status != 200)
    {
        std::println("[SNIPE] Latest version endpoint returned {} for {}{}", res->status, hostUrl, latestPath);
        return false;
    }

    resolvedVersion = TrimWhitespace(res->body);
    if (resolvedVersion.empty())
    {
        std::println("[SNIPE] Latest version endpoint was empty for {}{}", hostUrl, latestPath);
        return false;
    }

    return true;
}

std::string QuoteForShell(const std::string& value)
{
    return '"' + value + '"';
}

bool IsPackageBeingInstalled(const std::string& packageName)
{
    return std::find(gInstallingPackages.begin(), gInstallingPackages.end(), packageName) != gInstallingPackages.end();
}

void MarkPackageInstalling(const std::string& packageName)
{
    gInstallingPackages.push_back(packageName);
}

void UnmarkPackageInstalling(const std::string& packageName)
{
    auto it = std::find(gInstallingPackages.begin(), gInstallingPackages.end(), packageName);
    if (it != gInstallingPackages.end())
    {
        gInstallingPackages.erase(it);
    }
}

bool RunCommandInDirectory(const fs::path& directory, const std::string& command)
{
    std::error_code ec;
    fs::path previous = fs::current_path(ec);
    if (ec)
    {
        std::println("[SNIPE] Failed to inspect current working directory: {}", ec.message());
        return false;
    }

    fs::current_path(directory, ec);
    if (ec)
    {
        std::println("[SNIPE] Failed to change to {}: {}", directory.string(), ec.message());
        return false;
    }

    int rc = std::system(command.c_str());

    fs::current_path(previous, ec);
    if (ec)
    {
        std::println("[SNIPE] Failed to restore working directory: {}", ec.message());
    }

    return rc == 0;
}

std::string FindPythonExecutable()
{
    const char* envPython = std::getenv("SNIPE_PYTHON");
    if (envPython != nullptr && envPython[0] != '\0')
    {
        return envPython;
    }

#if defined(_WIN32)
    const char* candidates[] = {"py", "python", "python3"};
#else
    const char* candidates[] = {"python3", "python"};
#endif

    for (const char* candidate : candidates)
    {
#ifdef _WIN32
        std::string check = std::string(candidate) + " --version >nul 2>&1";
#else
        std::string check = std::string(candidate) + " --version >/dev/null 2>&1";
#endif
        if (std::system(check.c_str()) == 0)
        {
            return candidate;
        }
    }

    return {};
}

bool RunSetupScript(const fs::path& packageDir)
{
    fs::path setupPath = packageDir / "setup.py";
    if (!fs::exists(setupPath))
    {
        std::println("[SNIPE] No setup.py found in {}.", packageDir.string());
        return true;
    }

    std::string python = FindPythonExecutable();
    if (python.empty())
    {
        std::println("[SNIPE] Python interpreter not found. Cannot run setup.py.");
        return false;
    }

    std::string command = "PKGDIR=" + QuoteForShell(packageDir.string()) + " " + QuoteForShell(python) + " setup.py install";
    std::println("[SNIPE] Running setup.py in {}", packageDir.string());
    return RunCommandInDirectory(packageDir, command);
}

bool InstallDependencies(const fs::path& packageDir)
{
    fs::path depsPath = packageDir / ".deps";
    if (!fs::exists(depsPath))
    {
        return true;
    }

    std::ifstream infile(depsPath);
    if (!infile.is_open())
    {
        std::println("[SNIPE] Failed to open .deps file.");
        return false;
    }

    std::string line;
    while (std::getline(infile, line))
    {
        if (line.empty() || line.starts_with('#'))
        {
            continue;
        }

        std::string repoToken;
        std::string packageToken;
        std::string versionToken;

        std::string::size_type firstSep = line.find("--");
        if (firstSep == std::string::npos)
        {
            continue;
        }

        std::string::size_type secondSep = line.find("--", firstSep + 2);
        if (secondSep == std::string::npos)
        {
            continue;
        }

        repoToken = line.substr(0, firstSep);
        packageToken = line.substr(firstSep + 2, secondSep - firstSep - 2);
        versionToken = line.substr(secondSep + 2);

        auto trim = [](std::string value) {
            const auto first = value.find_first_not_of(" \t");
            if (first == std::string::npos)
            {
                return std::string{};
            }
            const auto last = value.find_last_not_of(" \t");
            return value.substr(first, last - first + 1);
        };

        repoToken = trim(repoToken);
        packageToken = trim(packageToken);
        versionToken = trim(versionToken);

        if (repoToken.empty() || packageToken.empty() || versionToken.empty())
        {
            continue;
        }

        std::string depName = packageToken;
        std::string depVersion = versionToken;

        if (IsPackageBeingInstalled(depName))
        {
            std::println("[SNIPE] Skipping circular dependency {} {}", depName, depVersion);
            continue;
        }

        std::println("[SNIPE] Installing dependency {} {}", depName, depVersion);
        MarkPackageInstalling(depName);
        int depResult = InstallPackage(depName, depVersion);
        UnmarkPackageInstalling(depName);

        if (depResult != 0)
        {
            std::println("[SNIPE] Dependency install failed for {} {}", depName, depVersion);
            return false;
        }
    }

    return true;
}

std::vector<std::string> ReadRepositoryList()
{
    std::vector<std::string> repo;
    fs::path packrepoFile = GetSnipeInstallDir() / "packrepo.lst";

    std::ifstream infile(packrepoFile);
    if (!infile.is_open())
    {
        std::println("[SNIPE] Cannot open repository file.");
        return repo;
    }

    std::string currentLine;
    while (std::getline(infile, currentLine))
    {
        if (currentLine.empty() || currentLine.starts_with('#'))
        {
            continue;
        }

        std::stringstream ss(currentLine);
        std::string trimmed;
        ss >> trimmed;
        if (!trimmed.empty())
        {
            repo.push_back(trimmed);
        }
    }

    return repo;
}

bool ParseRepository(const std::string& repoEntry, std::string& host, std::string& basePath)
{
    std::string working = repoEntry;
    if (working.rfind("https://", 0) == 0)
    {
        working = working.substr(8);
    }
    else if (working.rfind("http://", 0) == 0)
    {
        working = working.substr(7);
    }

    host = working;
    basePath = "/";

    const auto slashPos = host.find('/');
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

    return !host.empty();
}

} // namespace

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

    bool writeError = false;
    auto res = cli.Get(path.c_str(),
        [&](const char* data, size_t dataLength)
        {
            if (writeError)
            {
                return false;
            }

            outFile.write(data, static_cast<std::streamsize>(dataLength));
            if (!outFile)
            {
                writeError = true;
                return false;
            }

            return true;
        });

    outFile.close();

    if (!res)
    {
        std::println("[SNIPE] Network Error: {}", static_cast<int>(res.error()));
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

    if (writeError)
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

int UntargzArchive(const fs::path& archivePath, const fs::path& destinationDir)
{
    if (!fs::exists(archivePath))
    {
        std::println("[SNIPE] Archive does not exist: {}", archivePath.string());
        return -1;
    }

    std::error_code ec;
    fs::create_directories(destinationDir, ec);
    if (ec)
    {
        std::println("[SNIPE] Failed to create extraction directory {}: {}", destinationDir.string(), ec.message());
        return -2;
    }

    std::string python = FindPythonExecutable();
    if (python.empty())
    {
        std::println("[SNIPE] Python interpreter not found. Cannot extract archive.");
        return -4;
    }

    std::string script =
        "import os, sys, tarfile\n"
        "archive = sys.argv[1]\n"
        "destination = sys.argv[2]\n"
        "os.makedirs(destination, exist_ok=True)\n"
        "with tarfile.open(archive, 'r:gz') as handle:\n"
        "    handle.extractall(destination)\n";

    std::string command = QuoteForShell(python) + " -c " + QuoteForShell(script) + " " +
        QuoteForShell(archivePath.string()) + " " + QuoteForShell(destinationDir.string());

    int rc = std::system(command.c_str());
    if (rc != 0)
    {
        std::println("[SNIPE] Failed to extract archive with Python.");
        return -3;
    }

    return 0;
}

int DownloadPackage(const std::string& packageName, const std::string& packageVersion)
{
    if (IsPackageBeingInstalled(packageName))
    {
        std::println("[SNIPE] Package {} is already being installed; skipping recursive install.", packageName);
        return 0;
    }

    MarkPackageInstalling(packageName);

    std::vector<std::string> repo = ReadRepositoryList();
    if (repo.empty())
    {
        std::println("[SNIPE] No repositories configured.");
        return -2;
    }

    fs::path installRoot = GetSnipeInstallDir();
    fs::path destDir = installRoot / "cache";
    fs::create_directories(destDir);

    for (const auto& repoEntry : repo)
    {
        std::string host;
        std::string basePath;
        if (!ParseRepository(repoEntry, host, basePath))
        {
            continue;
        }

        std::string effectiveVersion = packageVersion;
        if (IsLatestVersionRequest(packageVersion))
        {
            if (!ResolveLatestVersion(host, basePath, packageName, effectiveVersion))
            {
                std::println("[SNIPE] Repository {} did not provide a .latest version for {}", repoEntry, packageName);
                continue;
            }
        }

        fs::path archivePath = destDir / (packageName + "-" + effectiveVersion + ".tar.gz");
        fs::path packageDir = installRoot / "packages" / (packageName + "-" + effectiveVersion);
        std::string packagePath = basePath + packageName + "/" + effectiveVersion + "/" + packageName + "-" + effectiveVersion + ".tar.gz";

        int rc = DownloadPackageArchive(host, packagePath, archivePath);
        if (rc != 0)
        {
            std::println("[SNIPE] Repository {} failed with code {}", repoEntry, rc);
            continue;
        }

        std::println("[SNIPE] Download complete. Extracting package...");
        if (UntargzArchive(archivePath, packageDir) != 0)
        {
            UnmarkPackageInstalling(packageName);
            return -4;
        }

        if (!InstallDependencies(packageDir))
        {
            UnmarkPackageInstalling(packageName);
            return -5;
        }

        if (RunSetupScript(packageDir))
        {
            UnmarkPackageInstalling(packageName);
            std::println("[SNIPE] Installation complete for {} {}.", packageName, effectiveVersion);
            return 0;
        }

        UnmarkPackageInstalling(packageName);
        std::println("[SNIPE] setup.py failed for {} {}.", packageName, effectiveVersion);
        return -6;
    }

    UnmarkPackageInstalling(packageName);
    std::println("[SNIPE] All repositories failed.");
    return -3;
}

int RemovePackage(const std::string& packageName, const std::string& packageVersion)
{
    fs::path installRoot = GetSnipeInstallDir();
    fs::path packageDir = installRoot / "packages" / (packageName + "-" + packageVersion);

    if (!fs::exists(packageDir) || !fs::is_directory(packageDir))
    {
        std::println("[SNIPE] Package {} {} is not installed.", packageName, packageVersion);
        return -1;
    }

    std::error_code ec;
    fs::remove_all(packageDir, ec);
    if (ec)
    {
        std::println("[SNIPE] Failed to remove package {} {}: {}", packageName, packageVersion, ec.message());
        return -2;
    }

    fs::path archivePath = installRoot / "cache" / (packageName + "-" + packageVersion + ".tar.gz");
    std::error_code archiveEc;
    fs::remove(archivePath, archiveEc);

    std::println("[SNIPE] Removed package {} {}.", packageName, packageVersion);
    return 0;
}

int ExecTool(const std::string& packageName, const std::string& packageVersion, const std::string& toolName, const std::string& arguments)
{
    fs::path installRoot = GetSnipeInstallDir();
    fs::path packageDir = installRoot / "packages" / (packageName + "-" + packageVersion);

    if (!fs::exists(packageDir) || !fs::is_directory(packageDir))
    {
        std::println("[SNIPE] Package {} {} is not installed.", packageName, packageVersion);
        return -1;
    }

    #ifdef _WIN32
    fs::path toolPath = (packageDir / (toolName + ".exe")).string();
    #else
    fs::path toolPath = (packageDir / toolName).string();
    #endif

    if (!fs::exists(toolPath) || !fs::is_regular_file(toolPath))
    {
        std::println("[SNIPE] Tool {} does not exist in package {} {}.", toolName, packageName, packageVersion);
        return -2;
    }

    std::string command = QuoteForShell(toolPath.string()) + " " + arguments;
    int rc = std::system(command.c_str());
    if (rc != 0)
    {
        std::println("[SNIPE] Tool execution failed with code {}", rc);
        return rc;
    }

    return 0;
}

int InstallPackage(const std::string& packageName, const std::string& packageVersion)
{
    return DownloadPackage(packageName, packageVersion);
}