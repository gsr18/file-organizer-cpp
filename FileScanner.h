#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class FileScanner {
public:
    // Scans a folder and returns all file paths found inside it
    std::vector<fs::path> scan(const std::string& folderPath);
};