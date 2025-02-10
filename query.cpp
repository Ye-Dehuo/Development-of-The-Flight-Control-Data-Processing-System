#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>

// 生成文件名
std::string generateFileName(const std::string& timestamp) {

    return "飞控数据_" + timestamp + ".txt";
}

// 读取指定时间段的数据
void queryData(const std::string& start, const std::string& end) {
    // 生成起始和结束的文件名
    std::string startFile = generateFileName(start);
    std::string endFile = generateFileName(end);

    std::vector<std::string> filesToRead; // 用于储存要读取的文件名

    // 遍历目录中的所有文件并选择时间范围内的文件
    for (const auto& entry : std::filesystem::directory_iterator("./")) {
        if (entry.path().filename().string() >= startFile && entry.path().filename().string() <= endFile) {
            filesToRead.push_back(entry.path().string());
        }
    }

    if (filesToRead.size() == 0) std::cout << "No files found!" << std::endl;

    // 遍历所选文件，读取数据
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
    std::cout << "请输入查询的起始时间（14位）：";
    std::cin >> startTimestamp;
    std::cout << "请输入查询的结束时间（14位）：";
    std::cin >> endTimestamp;

    queryData(startTimestamp, endTimestamp);
    return 0;
}
