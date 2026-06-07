#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::cout << "mk-2015 (C) allforall tools, fwipe tool\n";
	
	std::string filename = "GNU_IS-NOT-UNIX.like_linux";
	
	for (int i = 1; i < argc; i++)
	{
		std::string_view arg = std::string_view(argv[i]);
		if (arg == "-f" || arg == "--file") 
		{
			if (i + 1 < argc) 
			{
				filename = std::string(argv[++i]);
			}
		}
		
		if (arg == "-h" || arg == "--help")
		{
			std::cout << "Wipe tool:\n";
			std::cout << "\t-h or --help        := open help menu\n";
			std::cout << "\t-f or --file <FILE> := wipe a file (REQUIRED)\n";
		}
	}
	
    std::ofstream outFile(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);

    if (!outFile.is_open()) {
        std::cerr << "Error: File does not exist!\n";
        return 1;
    }

    std::streampos fileSize = outFile.tellp(); 
    
    outFile.seekp(0, std::ios::beg);           

    std::streamsize remainingBytes = static_cast<std::streamsize>(fileSize);

    std::vector<char> dummyData(1024, 0xFF); 

    if (remainingBytes > 0) {
        std::streamsize bytesToWrite = std::min(remainingBytes, static_cast<std::streamsize>(dummyData.size()));
        
        outFile.write(dummyData.data(), bytesToWrite);
        std::cout << "Wrote " << bytesToWrite << " bytes from 0x0 to the file ending.\n";
    }

    outFile.close();

    try {
        if (fs::remove(filename)) {
            std::cout << "File successfully deleted.\n";
        } else {
            std::cout << "File not found for deletion.\n";
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Deletion error: " << e.what() << "\n";
    }

    return 0;
}