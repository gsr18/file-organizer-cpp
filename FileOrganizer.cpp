#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "FileScanner.h"
#include "FileClassifier.h"
#include "FileMover.h"

// Forward declarations
void runOrganize(const std::string& folderPath);
void runUndo(const std::string& folderPath);

std::string getDateFolder(const fs::path& filePath) {
    auto ftime = fs::last_write_time(filePath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
    std::tm mt{};
    localtime_s(&mt, &tt);

    std::string months[] = {
        "January", "February", "March", "April",
        "May", "June", "July", "August",
        "September", "October", "November", "December"
    };

    std::ostringstream oss;
    oss << (1900 + mt.tm_year) << "-" << months[mt.tm_mon];
    return oss.str();
}

void runByDate(const std::string& folderPath) {
    FileScanner scanner;
    std::vector<fs::path> files = scanner.scan(folderPath);

    if (files.empty()) {
        std::cout << "No files found in that folder.\n";
        return;
    }

    std::map<std::string, int> dateCounts;
    for (const auto& file : files) {
        try {
            dateCounts[getDateFolder(file)]++;
        }
        catch (...) {
            dateCounts["Unknown"]++;
        }
    }

    std::cout << "\n--- Date Preview ---\n";
    for (const auto& [date, count] : dateCounts) {
        std::cout << "  " << std::left << std::setw(20) << date
            << " <-  " << count << " files\n";
    }
    std::cout << "\nTotal: " << files.size() << " files\n";

    std::cout << "\nOrganize by date? (yes/no): ";
    std::string answer;
    std::getline(std::cin, answer);
    if (answer != "yes") {
        std::cout << "Cancelled. No files were moved.\n";
        return;
    }

    FileMover mover;
    std::vector<MoveRecord> log;

    std::cout << "\nOrganizing by date...\n";
    for (const auto& file : files) {
        std::string dateFolder;
        try {
            dateFolder = getDateFolder(file);
        }
        catch (...) {
            dateFolder = "Unknown";
        }
        MoveRecord record = mover.moveFile(file, dateFolder, fs::path(folderPath));
        if (!record.to.empty()) log.push_back(record);
    }

    fs::path logFile = fs::path(folderPath) / "organizer_undo.log";
    std::ofstream logOut(logFile);
    for (const auto& record : log) {
        logOut << record.from.string() << "\n" << record.to.string() << "\n";
    }

    std::cout << "\n--- Done! ---\n";
    std::cout << "  Moved:   " << mover.getMovedCount() << " files\n";
    std::cout << "  Skipped: " << mover.getSkippedCount() << " files\n";
    std::cout << "  Undo log saved -> organizer_undo.log\n";
    std::cout << "  Run again and choose 'undo' to reverse.\n";
}

void runDryRun(const std::string& folderPath) {
    FileScanner scanner;
    std::vector<fs::path> files = scanner.scan(folderPath);

    if (files.empty()) {
        std::cout << "No files found in that folder.\n";
        return;
    }

    // Load custom rules if rules.txt exists
    FileClassifier classifier;
    fs::path rulesFile = fs::path(folderPath) / "rules.txt";
    classifier.loadCustomRules(rulesFile.string());

    std::map<std::string, int> categoryCounts;

    std::cout << "\n--- Dry Run Preview ---\n";
    std::cout << std::left
        << std::setw(40) << "File"
        << " ->  "
        << "Destination\n";
    std::cout << std::string(60, '-') << "\n";

    for (const auto& file : files) {
        std::string category = classifier.classify(file);
        std::string filename = file.filename().string();
        categoryCounts[category]++;

        if (filename.length() > 37) {
            filename = filename.substr(0, 34) + "...";
        }

        std::cout << std::left
            << std::setw(40) << filename
            << " ->  "
            << category << "/\n";
    }

    std::cout << std::string(60, '-') << "\n";
    std::cout << "\n--- Summary ---\n";
    for (const auto& [category, count] : categoryCounts) {
        std::cout << "  " << std::left << std::setw(12) << category
            << " :  " << count << " files\n";
    }
    std::cout << "\nTotal: " << files.size() << " files\n";

    std::cout << "\nDo you want to organize now? (yes/no): ";
    std::string answer;
    std::getline(std::cin, answer);
    if (answer == "yes") {
        runOrganize(folderPath);
    }
    else {
        std::cout << "No files were moved.\n";
    }
}

void runUndo(const std::string& folderPath) {
    fs::path logFile = fs::path(folderPath) / "organizer_undo.log";

    if (!fs::exists(logFile)) {
        std::cout << "No undo log found in that folder.\n";
        return;
    }

    std::ifstream logIn(logFile);
    std::string fromStr, toStr;
    int restoredCount = 0;

    std::cout << "Undoing...\n";
    while (std::getline(logIn, fromStr) && std::getline(logIn, toStr)) {
        try {
            if (fs::exists(toStr)) {
                fs::rename(toStr, fromStr);
                restoredCount++;
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cout << "  Warning: " << e.what() << "\n";
        }
    }
    logIn.close();
    fs::remove(logFile);

    // Remove empty category folders
    std::vector<std::string> categories = {
        "Images", "Videos", "Documents", "Music",
        "Archives", "Programs", "Code", "Others", "Design"
    };
    for (const auto& category : categories) {
        fs::path categoryFolder = fs::path(folderPath) / category;
        if (fs::exists(categoryFolder) && fs::is_empty(categoryFolder)) {
            fs::remove(categoryFolder);
        }
    }

    // Remove empty date folders
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (fs::is_directory(entry) && fs::is_empty(entry)) {
            fs::remove(entry);
        }
    }

    std::cout << "--- Undo complete! ---\n";
    std::cout << "  Restored: " << restoredCount << " files\n";
    std::cout << "  Empty folders cleaned up.\n";
}

void runOrganize(const std::string& folderPath) {
    FileScanner scanner;
    std::vector<fs::path> files = scanner.scan(folderPath);
    std::cout << "\nFound " << files.size() << " files\n";

    if (files.empty()) {
        std::cout << "No files found in that folder.\n";
        return;
    }

    // Load custom rules if rules.txt exists
    FileClassifier classifier;
    fs::path rulesFile = fs::path(folderPath) / "rules.txt";
    classifier.loadCustomRules(rulesFile.string());

    std::map<std::string, int> categoryCounts;
    for (const auto& file : files) {
        categoryCounts[classifier.classify(file)]++;
    }

    std::cout << "\n--- Preview ---\n";
    for (const auto& [category, count] : categoryCounts) {
        std::cout << "  " << category << "/  <-  " << count << " files\n";
    }

    std::cout << "\nOrganize these files? (yes/no): ";
    std::string answer;
    std::getline(std::cin, answer);
    if (answer != "yes") {
        std::cout << "Cancelled. No files were moved.\n";
        return;
    }

    FileMover mover;
    std::vector<MoveRecord> log;

    std::cout << "\nOrganizing...\n";
    for (const auto& file : files) {
        std::string category = classifier.classify(file);
        MoveRecord record = mover.moveFile(file, category, fs::path(folderPath));
        if (!record.to.empty()) log.push_back(record);
    }

    fs::path logFile = fs::path(folderPath) / "organizer_undo.log";
    std::ofstream logOut(logFile);
    for (const auto& record : log) {
        logOut << record.from.string() << "\n" << record.to.string() << "\n";
    }

    std::cout << "\n--- Done! ---\n";
    std::cout << "  Moved:   " << mover.getMovedCount() << " files\n";
    std::cout << "  Skipped: " << mover.getSkippedCount() << " files\n";
    std::cout << "  Undo log saved -> organizer_undo.log\n";
    std::cout << "  Run again and choose 'undo' to reverse.\n";
}

int main() {
    std::cout << "=== File Organizer ===\n";
    std::cout << "What do you want to do?\n";
    std::cout << "  organize  ->  sort files by type\n";
    std::cout << "  bydate    ->  sort files by date\n";
    std::cout << "  dryrun    ->  preview without moving\n";
    std::cout << "  undo      ->  reverse last operation\n";
    std::cout << "\nChoice: ";
    std::string mode;
    std::getline(std::cin, mode);

    std::string folderPath;
    std::cout << "Enter folder path: ";
    std::getline(std::cin, folderPath);

    if (mode == "organize") {
        runOrganize(folderPath);
    }
    else if (mode == "bydate") {
        runByDate(folderPath);
    }
    else if (mode == "dryrun") {
        runDryRun(folderPath);
    }
    else if (mode == "undo") {
        runUndo(folderPath);
    }
    else {
        std::cout << "Unknown command.\n";
    }

    return 0;
}