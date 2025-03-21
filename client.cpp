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
    float pitch;  // pitch angle
    float roll;   // roll angle
    float yaw;    // yaw angle
    unsigned long timestamp;  // timestamp(ms)
};

// Generate data
float generateRandomAngle(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

// Collect data
void collectData(std::vector<FlightData>& dataBuffer, int numSamples, int interval_ms) {
    srand(time(0));  // Use current time as the random seed
    unsigned long startTime = static_cast<unsigned long>(time(0)) * 1000; // Current time (milliseconds)

    for (int i = 0; i < numSamples; ++i) {
        FlightData data;
        data.pitch = generateRandomAngle(-180.0f, 180.0f);
        data.roll = generateRandomAngle(-180.0f, 180.0f);
        data.yaw = generateRandomAngle(-180.0f, 180.0f);
        data.timestamp = startTime + i * interval_ms;

        dataBuffer.push_back(data);

        // Simulate a time interval of 100 milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
    }
}

// Query the number of collected data
int getDataCount(const std::vector<FlightData> dataBuffer){
    return dataBuffer.size();
}

// Get the data element content at the specified index
FlightData getDataAtIndex(const std::vector<FlightData> dataBuffer, int index){
    if (index >= 0 && index < dataBuffer.size()){
        return dataBuffer[index];
    } else {
        std::cerr << "Error: Index out of range!" << std::endl;
        // Return an empty FlightData
        return FlightData{0.0f,0.0f,0.0f,0};
    }
}

// Client function, connects to the server and sends data
void startClient(const std::string& serverIp, const int& serverPort, int numSamples, int interval_ms){
    int sockfd;
    struct  sockaddr_in serverAddr;
    int retryCount = 0;
    bool isConnected = false;

    while(!isConnected){
        // Try to connect to the server, retry up to 5 times
        // Create socket
        while(retryCount < 5 && !isConnected){
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                std::cerr << "Error: Socket creation failed!" << std::endl;
                return;
            }

            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(serverPort);
            serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());

            if(connect(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == 0 ){
                std::cout << "Server connection succeed!" << std::endl;
                isConnected = true;
            }
            else {
                std::cerr << "Error: Server connection failed! Retrying..." << std::endl;
                close(sockfd);
                retryCount++;
                sleep(3); // Wait 3 seconds before retrying
            }
        }

        if (!isConnected){
            std::cerr << "Error: Server connection failed after 5 retryings! Exiting..." << std::endl;
            return;
        }

        std::vector<FlightData> dataBuffer; // Used to store a batch of data (10 sets)

        while(isConnected){
            // Collect data
            collectData(dataBuffer, numSamples, interval_ms); // Data collection completed for a batch

            if (dataBuffer.size() >= 10){
                // Get the latest 10 sets of data
                std::vector<FlightData> dataToSend(dataBuffer.end()-10, dataBuffer.end());
                
                // Send data to the server
                for (const auto& data : dataToSend) {

                    std::string dataStr = std::to_string(data.pitch) + ","
                                        + std::to_string(data.roll) + ","
                                        + std::to_string(data.yaw) + ","
                                        + std::to_string(data.timestamp);

                    if (send(sockfd, dataStr.c_str(), dataStr.length(), MSG_NOSIGNAL) < 0) {
                        std::cerr << "Send failed with errno: " << errno << std::endl;
                        isConnected = false;
                        break;  // Exit the current data sending loop, reattempt data collection and sending
                    }

                    sleep(1); // Non-blocking send() mode, giving the server's recv() some time to receive, otherwise, it might cause data confusion

                }
                
                // Clear the data already sent in the buffer
                dataBuffer.clear();
                
                // Send a batch of data every 5 seconds
                if(isConnected) sleep(5);
            }
        }

        close(sockfd);   
    }
}

int main() {
    std::vector<FlightData> flightDataBuffer;

    // Set the number of data samples and the time interval
    int numSamples = 10;      // Collect 10 sets of data
    int interval_ms = 100;    // Collect data every 100 milliseconds
    std::string serverIp = "127.0.0.1";  // Server IP address
    int serverPort = 12345;  // Server port number

    int index = 4; // Data index
   
    // collectData(flightDataBuffer, numSamples, interval_ms);

    // int dataCount = getDataCount(flightDataBuffer);

    // FlightData dataAtIndex = getDataAtIndex(flightDataBuffer, index);

    startClient(serverIp, serverPort, numSamples, interval_ms);

    return 0;
}

