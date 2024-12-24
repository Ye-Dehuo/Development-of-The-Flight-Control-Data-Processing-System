#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

struct FlightData {
    float pitch;  // 俯仰角
    float roll;   // 滚转角
    float yaw;    // 偏航角
    unsigned long timestamp;  // 时间戳（毫秒）
};

// 生成数据
float generateRandomAngle(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

// 采集数据
void collectData(std::vector<FlightData>& dataBuffer, int numSamples, int interval_ms) {
    srand(time(0));  // 用当前时间作为随机种子
    unsigned long startTime = static_cast<unsigned long>(time(0)) * 1000; // 当前时间（毫秒）

    for (int i = 0; i < numSamples; ++i) {
        FlightData data;
        data.pitch = generateRandomAngle(-180.0f, 180.0f);
        data.roll = generateRandomAngle(-180.0f, 180.0f);
        data.yaw = generateRandomAngle(-180.0f, 180.0f);
        data.timestamp = startTime + i * interval_ms;

        dataBuffer.push_back(data);

        // 模拟每100毫秒的时间间隔
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
}

// 查询已采集数据数量
int getDataCount(const std::vector<FlightData> dataBuffer){
    return dataBuffer.size();
}

// 获取指定索引位置的数据元素内容
FlightData getDataAtIndex(const std::vector<FlightData> dataBuffer, int index){
    if (index >= 0 && index < dataBuffer.size()){
        return dataBuffer[index];
    } else {
        std::cerr << "Error: Index out of range!" << std::endl;
        // 返回一个空的 FlightData
        return FlightData{0.0f,0.0f,0.0f,0};
    }
}

// 客户端函数，连接到服务器并发送数据
void client(const std::string& serverIp, const int& serverPort, int numSamples, int interval_ms){
    int sockfd;
    struct  sockaddr_in serverAddr;
    int retryCount = 0;
    bool isConnected = false;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());

// 尝试连接服务器，最多重试 5 次
// 创建套接字
while(retryCount < 5 && !isConnected){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Error: Socket creation failed!" << std::endl;
        return;
    }

    if(connect(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr) == 0)){
        std::cout << "Server connection succeed!" << std::endl;
        isConnected = true;
    }
    else {
        std::cerr << "Error: Server connection failed! Retrying..." << std::endl;
        close(sockfd);
        retryCount++;
        if (retryCount < 5){
            sleep(3); // 等待 3 秒再尝试重新连接
        }
    }
}

if (!isConnected){
    std::cerr << "Error: Server connection failed after 5 retryings! Exiting..." << std::endl;
    return;
}

std::vector<FlightData> dataBuffer;
while(true){
    // 采集数据
    collectData(dataBuffer, numSamples, interval_ms);

    if (dataBuffer.size() > 10){
        // 获取最新的10组数据
        std::vector<FlightData> dataToSend(dataBuffer.end()-10, dataBuffer.end());
        
        // 将数据发送到服务器
            for (const auto& data : dataToSend) {
                std::string dataStr = std::to_string(data.pitch) + ","
                                    + std::to_string(data.roll) + ","
                                    + std::to_string(data.yaw) + ","
                                    + std::to_string(data.timestamp);

                if (send(sockfd, dataStr.c_str(), dataStr.length(), 0) < 0) {
                    std::cerr << "Error: Failed to send data. Reconnecting..." << std::endl;
                    break;  // 跳出当前的发送数据循环，重新发送
                }
            }
        
        // 清空缓冲区中已发送的数据
        dataBuffer.clear();
            
        // 每 5 秒发送一次数据
        sleep(5);
    }
}

close(sockfd);   
}

int main() {
    std::vector<FlightData> flightDataBuffer;

    // 设置采集的数据条数和时间间隔
    int numSamples = 10;      // 采集 10 次数据
    int interval_ms = 100;    // 每 100 毫秒采集一次数据
    std::string serverIp = "127.0.0.1";  // 服务器 IP 地址
    int serverPort = 12345;  // 服务器端口号

    int index = 4; // 数据索引
   
    // collectData(flightDataBuffer, numSamples, interval_ms);

    // int dataCount = getDataCount(flightDataBuffer);

    // FlightData dataAtIndex = getDataAtIndex(flightDataBuffer, index);

    client(serverIp, serverPort, numSamples, interval_ms);

    return 0;
}
