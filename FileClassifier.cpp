#include "FileClassifier.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

FileClassifier::FileClassifier() {
    loadBuiltInRules();
}

void FileClassifier::loadBuiltInRules() {
    // Images
    extensionMap[".jpg"] = "Images";
    extensionMap[".jpeg"] = "Images";
    extensionMap[".png"] = "Images";
    extensionMap[".gif"] = "Images";
    extensionMap[".bmp"] = "Images";
    extensionMap[".svg"] = "Images";
    extensionMap[".webp"] = "Images";

    // Videos
    extensionMap[".mp4"] = "Videos";
    extensionMap[".mkv"] = "Videos";
    extensionMap[".avi"] = "Videos";
    extensionMap[".mov"] = "Videos";
    extensionMap[".wmv"] = "Videos";

    // Music
    extensionMap[".mp3"] = "Music";
    extensionMap[".wav"] = "Music";
    extensionMap[".flac"] = "Music";
    extensionMap[".aac"] = "Music";

    // Documents
    extensionMap[".pdf"] = "Documents";
    extensionMap[".doc"] = "Documents";
    extensionMap[".docx"] = "Documents";
    extensionMap[".txt"] = "Documents";
    extensionMap[".pptx"] = "Documents";
    extensionMap[".xlsx"] = "Documents";
    extensionMap[".csv"] = "Documents";

    // Archives
    extensionMap[".zip"] = "Archives";
    extensionMap[".rar"] = "Archives";
    extensionMap[".7z"] = "Archives";
    extensionMap[".tar"] = "Archives";
    extensionMap[".gz"] = "Archives";

    // Programs
    extensionMap[".exe"] = "Programs";
    extensionMap[".msi"] = "Programs";
    extensionMap[".dll"] = "Programs";

    // Code
    extensionMap[".cpp"] = "Code";
    extensionMap[".h"] = "Code";
    extensionMap[".py"] = "Code";
    extensionMap[".js"] = "Code";
    extensionMap[".html"] = "Code";
    extensionMap[".css"] = "Code";
    extensionMap[".java"] = "Code";
}

void FileClassifier::loadCustomRules(const std::string& rulesFilePath) {
    std::ifstream file(rulesFilePath);

    if (!file.is_open()) {
        // No rules file found — silently use built-in rules
        return;
    }

    std::string line;
    int loaded = 0;

    while (std::getline(file, line)) {
        // Skip empty lines and comments (lines starting with #)
        if (line.empty() || line[0] == '#') continue;

        // Parse "  .ext  ->  Category  "
        auto arrowPos = line.find("->");
        if (arrowPos == std::string::npos) continue;

        std::string ext = line.substr(0, arrowPos);
        std::string category = line.substr(arrowPos + 2);

        // Trim whitespace from both sides
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            s.erase(s.find_last_not_of(" \t\r\n") + 1);
            };
        trim(ext);
        trim(category);

        // Convert extension to lowercase
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (!ext.empty() && !category.empty()) {
            extensionMap[ext] = category;  // overrides built-in if same ext
            loaded++;
        }
    }

    std::cout << "  Loaded " << loaded << " custom rules from rules.txt\n";
}

std::string FileClassifier::classify(const fs::path& filePath) {
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    auto it = extensionMap.find(ext);
    if (it != extensionMap.end()) {
        return it->second;
    }

    return "Others";
}