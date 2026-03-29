#pragma once
#include <string>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

class FileClassifier {
public:
    FileClassifier();           // loads built-in rules
    void loadCustomRules(const std::string& rulesFilePath);  // loads user rules.txt
    std::string classify(const fs::path& filePath);

private:
    std::map<std::string, std::string> extensionMap;
    void loadBuiltInRules();
};