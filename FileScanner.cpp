#include "FileScanner.h"
#include <iostream>

std::vector<fs::path> FileScanner::scan(const std::string& folderPath) {
    std::vector<fs::path> files;

    // Check if the folder actually exists
    if (!fs::exists(folderPath)) {
        std::cout << "Error: Folder not found -> " << folderPath << "\n";
        return files;
    }

    // Walk through every file in the folder
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (fs::is_regular_file(entry)) {  // skip subfolders for now
            files.push_back(entry.path());
        }
    }

    return files;
}