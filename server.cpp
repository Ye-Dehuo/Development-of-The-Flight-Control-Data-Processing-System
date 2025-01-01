#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <sstream>
#include <limits>
#include <fstream>
#include <chrono>
#include <iomanip>

struct FlightData {
    float pitch;  // 俯仰角
    float roll;   // 滚转角
    float yaw;    // 偏航角
    unsigned long timestamp;  // 时间戳（毫秒）
};

// 生成文件名
std::string generateFileName(const unsigned long& timestamp) {

   unsigned long timestamp_s = timestamp / 1000; // 时间戳单位ms转换为s
  
     // 将时间戳转换为time_t，然后转换为tm结构
    std::time_t time = static_cast<time_t>(timestamp_s);
    std::tm *ptm = std::localtime(&time);

     // 使用std::ostringstream与std::put_time来格式化日时间
    std::ostringstream oss;
    oss << std::put_time(ptm, "%Y%m%d%H%M%S");

    // 从输出字符串流中获取格式化后的字符串,并输出文件名
    std::string timeStr = oss.str();
    return "飞控数据_" + timeStr + ".txt";
    }

// 将字符串数据转换为FlightData数据
FlightData convertData(const std::string data){
    std::istringstream ss(data);
    std::string token;
    FlightData fd;

    std::getline(ss, token, ',');
    fd.pitch = std::stof(token);

    std::getline(ss, token, ',');
    fd.roll = std::stof(token);

    std::getline(ss, token, ',');
    fd.yaw = std::stof(token);

    std::getline(ss, token);
    fd.timestamp = std::stoul(token);

    return fd;
}

// 数据处理
void processData(const std::vector<std::string>& dataString) {
    if (dataString.empty()) return;

    std::vector<FlightData> fdBuffer;

    for(const auto& data : dataString){
        FlightData fd = convertData(data);
        fdBuffer.push_back(fd);
    }

    std::string filename = generateFileName(fdBuffer.front().timestamp);
    std::ofstream file(filename);

// 初始化统计值
    float maxPitch = std::numeric_limits<float>::lowest();
    float minPitch = std::numeric_limits<float>::max();
    float sumPitch = 0.0;

    float maxRoll = std::numeric_limits<float>::lowest();
    float minRoll = std::numeric_limits<float>::max();
    float sumRoll = 0.0;

    float maxYaw = std::numeric_limits<float>::lowest();
    float minYaw = std::numeric_limits<float>::max();
    float sumYaw = 0.0;

    // 计算最大最小值
    for (const auto& entry : fdBuffer) {
        maxPitch = std::max(maxPitch, entry.pitch);
        minPitch = std::min(minPitch, entry.pitch);
        sumPitch += entry.pitch;

        maxRoll = std::max(maxRoll, entry.roll);
        minRoll = std::min(minRoll, entry.roll);
        sumRoll += entry.roll;

        maxYaw = std::max(maxYaw, entry.yaw);
        minYaw = std::min(minYaw, entry.yaw);
        sumYaw += entry.yaw;

    // 记录原始数据
        file <<"Raw data: " << std::endl;
        file << entry.timestamp << " " << entry.pitch << " " << entry.roll << " " << entry.yaw << std::endl;
     }

    // 计算平均值
    float avgPitch = sumPitch / fdBuffer.size();
    float avgRoll = sumRoll / fdBuffer.size();
    float avgYaw = sumYaw / fdBuffer.size();

    // 记录统计数据
    file << "Statistics data: " <<std::endl;
    file << "Average: " << avgPitch << " " << avgRoll << " " << avgYaw << std::endl;
    file << "Max data: " << maxPitch << " " << maxRoll << " " << maxYaw << std::endl;
    file << "Min data: " << minPitch << " " << minRoll << " " << minYaw << std::endl;

    file.close(); // 关闭文件
}

// 校验数据格式和数量
bool validateData(const std::vector<std::string>& dataStrings, int expectedDataCount) {
    if (dataStrings.size() != expectedDataCount) {
        std::cerr << "Error: Data count mismatch. Expected " << expectedDataCount << ", but received " << dataStrings.size() << std::endl;
        return false;
    }

    // 校验每一条数据格式
    for (const auto& data : dataStrings) {
        std::istringstream ss(data);
        std::string token;
        int count = 0;
        while (std::getline(ss, token, ',')) { // 分割接收到的原始数据字符串
            count++;
            // 检查每个数据项是否符合预期格式
            if (count == 1 || count == 2 || count == 3) {
                try {
                    std::stof(token);  // 检查是否为浮点数
                } catch (const std::exception&) {
                    std::cerr << "Error: Invalid data format!" << count << ": " << token << std::endl;
                    return false;
                }
            } else if (count == 4) {
                try {
                    std::stol(token);  // 检查时间戳是否为整数
                } catch (const std::exception&) {
                    std::cerr << "Error: Invalid timestamp format: " << token << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}

// 服务器函数：接收客户端发送的数据并校验、存储与处理
void receiveDataFromClient(int newSock) {
    std::vector<std::string> dataStrings;  // 用于存储接收到的单批数据
    char buffer[1024];  // 接收单组数据的临时缓冲区

    int expectedDataCount = 10;  // 期望每次接收到的数据数量

    while (true) {

        while(dataStrings.size() < 10){
        memset(buffer, 0, sizeof(buffer));// 每次接收前，临时缓冲区初始化为0

        // 接收数据
        int bytesReceived = recv(newSock, buffer, sizeof(buffer), 0);
        if (bytesReceived < 0) {
            std::cerr << "Error or connection closed by client!" << std::endl;
            break;
        }

        std::string data(buffer); 
        dataStrings.push_back(data);

        std::cout << dataStrings.size() << " set of data has been received" << std::endl; // 打印本组数据已接收数量
        if (dataStrings.size() == 10) std::cout << "This batch of data has been received" << std::endl << std::endl;
    }

        // 校验数据
        if (validateData(dataStrings, expectedDataCount)) {
            
        // 数据校验通过，存储数据到本地内存（这里只是简单打印）
            std::cout << "Data received and validated. Storing data..." << std::endl << std::endl ;

            for (const auto& data : dataStrings) {
                std::cout << "Received data: " << data << std::endl;
                std::cout << std::endl;
            }

        // 数据处理
        processData(dataStrings);

            // 清空数据字符串，准备接收下一批数据
            dataStrings.clear();
        } else {
            // 校验失败，向客户端发送错误信息并要求重新发送
            std::string errorMessage = "Error: Invalid data format or data count mismatch. Please resend data.";
            send(newSock, errorMessage.c_str(), errorMessage.length(), 0);
        }
    }

    close(newSock);
}

// 服务器主函数：监听并接受客户端连接
void startServer(const std::string& serverIp, const int& serverPort) {
    int serverSock;
    struct sockaddr_in serverAddr;

    // 创建套接字
    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error: Socket creation failed!" << std::endl;
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
    serverAddr.sin_port = htons(serverPort);

    // 绑定套接字
    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Socket binding failed!" << std::endl;
        return;
    }

    // 开始监听
    if (listen(serverSock, 1) < 0) {
        std::cerr << "Error: Listen failed!" << std::endl;
        return;
    }

    std::cout << "Server is listening on " << serverIp << ":" << serverPort << std::endl;

    // 等待并接受客户端连接
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        int newSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (newSock < 0) {
            std::cerr << "Error: Accept failed! Retrying..." << std::endl;
            continue;
        }

        std::cout << "Client connected!" << std::endl;
        
        // 从客户端接收数据
        receiveDataFromClient(newSock);
    }

    close(serverSock);
}

int main() {
    std::string serverIp = "127.0.0.1";  // 服务器 IP 地址
    int serverPort = 12345;  // 服务器端口号

    // 启动服务器，等待客户端连接并接收数据
    startServer(serverIp, serverPort);

    return 0;
}
