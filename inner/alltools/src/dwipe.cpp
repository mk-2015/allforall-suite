#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;

void wipeFileContents(const fs::path& filePath) {
    std::ofstream outFile(filePath, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);

    if (!outFile.is_open()) {
        return; 
    }

    std::streampos fileSize = outFile.tellp(); 
    outFile.seekp(0, std::ios::beg);           

    std::streamsize remainingBytes = static_cast<std::streamsize>(fileSize);

    std::vector<char> zeroBuffer(4096, 0x00); 

    while (remainingBytes > 0) {
        std::streamsize toWrite = std::min(remainingBytes, static_cast<std::streamsize>(zeroBuffer.size()));
        outFile.write(zeroBuffer.data(), toWrite);
        remainingBytes -= toWrite;
    }

    outFile.close();
}

int main(int argc, char* argv[]) 
{
    std::cout << "mk-2015 (C) allforall tools, dwipe\n";
	fs::path targetDir = "unknown";

	for(int i = 1; i < argc; i++)
	{
		std::string_view arg = std::string_view(argv[i]);
		if (arg == "-d" || arg == "--dir")
		{
			if(i + 1 < argc) 
			{
				targetDir = fs::path(argv[++i]);
			}
		} else if (arg == "h" || arg == "--help")
		{
			std::cout << "DWipe tool:\n";
			std::cout << "\t-h or --help        := open help menu\n";
			std::cout << "\t-d or --dir <DIR>   := wipe a directory (REQUIRED)\n";
		}
	}

    if (!fs::exists(targetDir) || !fs::is_directory(targetDir)) {
        std::cerr << "Error: Target directory does not exist or is not a folder.\n";
        return 1;
    }

    std::cout << "Wiping contents of: " << targetDir << "\n";

    try {
        for (const auto& entry : fs::recursive_directory_iterator(targetDir)) {
            if (fs::is_regular_file(entry.path())) {
                wipeFileContents(entry.path());
            }
        }
        std::cout << "All files safely zeroed out.\n";
    } 
    catch (const fs::filesystem_error& e) {
        std::cerr << "Traversal error during overwrite: " << e.what() << "\n";
    }

    try {
        std::uintmax_t deletedCount = fs::remove_all(targetDir);
        std::cout << "Successfully deleted target directory and " << deletedCount << " items.\n";
    } 
    catch (const fs::filesystem_error& e) {
        std::cerr << "Deletion error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
