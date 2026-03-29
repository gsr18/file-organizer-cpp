#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// Stores the record of one file move (needed for undo later)
struct MoveRecord {
    fs::path from;
    fs::path to;
};

class FileMover {
public:
    // Moves a file into the correct category subfolder
    // Returns a MoveRecord so we can undo it later
    MoveRecord moveFile(const fs::path& filePath,
        const std::string& category,
        const fs::path& destinationRoot);

    // Getters
    int getMovedCount() { return movedCount; }
    int getSkippedCount() { return skippedCount; }

private:
    int movedCount = 0;
    int skippedCount = 0;

    // If a file with same name already exists, adds _1, _2 etc.
    fs::path resolveConflict(const fs::path& destination);
};