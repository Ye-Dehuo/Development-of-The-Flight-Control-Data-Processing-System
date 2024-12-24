#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <sstream>

struct FlightData {
    float pitch;  // 俯仰角
    float roll;   // 滚转角
    float yaw;    // 偏航角
    long timestamp;  // 时间戳（毫秒）
};

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
            // 检查每个数据项是否符合预期格式：浮点数
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

// 服务器函数：接收客户端发送的数据并校验
void receiveDataFromClient(int newSock) {
    std::vector<std::string> dataStrings;  // 用于存储接收到的原始数据字符串
    char buffer[1024];  // 接收数据的缓冲区

    int expectedDataCount = 10;  // 期望每次接收到的数据数量

    while (true) {
        memset(buffer, 0, sizeof(buffer));// 缓冲区初始化为0

        // 接收数据
        int bytesReceived = recv(newSock, buffer, sizeof(buffer), 0);
        if (bytesReceived < 0) {
            std::cerr << "Error: Failed to receive data!" << std::endl;
            break;
        }

        std::string data(buffer); 
        dataStrings.push_back(data);

        // 校验数据
        if (validateData(dataStrings, expectedDataCount)) {
            // 数据校验通过，存储数据到本地内存（这里只是简单打印）
            std::cout << "Data received and validated. Storing data..." << std::endl;

            for (const auto& data : dataStrings) {
                std::cout << "Received data: " << data << std::endl;
            }

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
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

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
    if (listen(serverSock, 3) < 0) {
        std::cerr << "Error: Listen failed!" << std::endl;
        return;
    }

    std::cout << "Server is listening on " << serverIp << ":" << serverPort << std::endl;

    // 等待并接收客户端连接
    while (true) {
        int newSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (newSock < 0) {
            std::cout << "Waiting accept..." << std::endl;
            continue;
        }

        std::cout << "Client connected!" << std::endl;
        
        // 处理客户端的数据接收
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
