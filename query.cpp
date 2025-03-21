#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

// Generate filename
std::string generateFileName(const std::string& timestamp) {

    return "FlightData_" + timestamp + ".txt";
}

// Query data for a specified time range
void queryData(const std::string& start, const std::string& end) {
    // Generate start and end filenames
    std::string startFile = generateFileName(start);
    std::string endFile = generateFileName(end);

    std::vector<std::string> filesToRead; // To store filenames to read

    // Iterate through all files in the directory and select files within the time range
    for (const auto& entry : std::filesystem::directory_iterator("./")) {
        if (entry.path().filename().string() >= startFile && entry.path().filename().string() <= endFile) {
            filesToRead.push_back(entry.path().string());
        }
    }

    if (filesToRead.size() == 0) std::cout << "No files found!" << std::endl;

    // Iterate through the selected files and read data
    for (const auto& filename : filesToRead) {
        std::ifstream file(filename);
        std::string line;
        while (getline(file, line)) {
            std::cout << line << std::endl;
        }
        std::cout << std::endl;
        file.close();
    }
}

int main() {
    std::string startTimestamp, endTimestamp;
    std::cout << "Please enter the start time for the query (14 digits): ";
    std::cin >> startTimestamp;
    std::cout << "Please enter the end time for the query (14 digits): ";
    std::cin >> endTimestamp;

    queryData(startTimestamp, endTimestamp);
    return 0;
}
