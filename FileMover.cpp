#include "FileMover.h"
#include <iostream>

fs::path FileMover::resolveConflict(const fs::path& destination) {
    if (!fs::exists(destination)) {
        return destination;  // no conflict, use as-is
    }

    // File already exists — add _1, _2, _3 until we find a free name
    fs::path stem = destination.stem();       // filename without extension
    fs::path ext = destination.extension();  // .jpg, .pdf etc.
    fs::path parentDir = destination.parent_path();

    int counter = 1;
    fs::path newDest;
    do {
        newDest = parentDir / (stem.string() + "_" + std::to_string(counter) + ext.string());
        counter++;
    } while (fs::exists(newDest));

    return newDest;
}

MoveRecord FileMover::moveFile(const fs::path& filePath,
    const std::string& category,
    const fs::path& destinationRoot) {
    MoveRecord record;
    record.from = filePath;

    try {
        // Create the category subfolder if it doesn't exist yet
        fs::path categoryFolder = destinationRoot / category;
        if (!fs::exists(categoryFolder)) {
            fs::create_directories(categoryFolder);
        }

        // Figure out destination path, handle conflicts
        fs::path destination = categoryFolder / filePath.filename();
        destination = resolveConflict(destination);

        // Actually move the file
        fs::rename(filePath, destination);

        record.to = destination;
        movedCount++;

    }
    catch (const fs::filesystem_error& e) {
        std::cout << "  Warning: Could not move "
            << filePath.filename() << " -> " << e.what() << "\n";
        skippedCount++;
    }

    return record;
}