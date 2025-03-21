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
#include <thread>

struct FlightData {
    float pitch;  // pitch angle
    float roll;   // roll angle
    float yaw;    // yaw angle
    unsigned long timestamp;  // timestamp (milliseconds)
};

// Generate filename
std::string generateFileName(const unsigned long& timestamp) {

   unsigned long timestamp_s = timestamp / 1000; // Convert timestamp from ms to s
  
     // Convert timestamp to time_t, then to tm structure
    std::time_t time = static_cast<time_t>(timestamp_s);
    std::tm *ptm = std::localtime(&time);

     // Use std::ostringstream and std::put_time to format time
    std::ostringstream oss;
    oss << std::put_time(ptm, "%Y%m%d%H%M%S");

    // Get the formatted string from the output stringstream and return the filename
    std::string timeStr = oss.str();
    return "FlightData_" + timeStr + ".txt";
}

// Convert string data to FlightData
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

// Data processing
void processData(const std::vector<std::string>& dataString) {
    if (dataString.empty()) return;

    std::vector<FlightData> fdBuffer;

    for(const auto& data : dataString){
        FlightData fd = convertData(data);
        fdBuffer.push_back(fd);
    }

    std::string filename = generateFileName(fdBuffer.front().timestamp);
    std::ofstream file(filename);

    // Initialize statistical values
    float maxPitch = std::numeric_limits<float>::lowest();
    float minPitch = std::numeric_limits<float>::max();
    float sumPitch = 0.0;

    float maxRoll = std::numeric_limits<float>::lowest();
    float minRoll = std::numeric_limits<float>::max();
    float sumRoll = 0.0;

    float maxYaw = std::numeric_limits<float>::lowest();
    float minYaw = std::numeric_limits<float>::max();
    float sumYaw = 0.0;

    // Calculate max and min values
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

        // Log raw data
        file << "Raw data: " << std::endl;
        file << entry.timestamp << " " << entry.pitch << " " << entry.roll << " " << entry.yaw << std::endl;
    }

    // Calculate averages
    float avgPitch = sumPitch / fdBuffer.size();
    float avgRoll = sumRoll / fdBuffer.size();
    float avgYaw = sumYaw / fdBuffer.size();

    // Log statistical data
    file << "Statistics data: " << std::endl;
    file << "Average: " << avgPitch << " " << avgRoll << " " << avgYaw << std::endl;
    file << "Max data: " << maxPitch << " " << maxRoll << " " << maxYaw << std::endl;
    file << "Min data: " << minPitch << " " << minRoll << " " << minYaw << std::endl;

    file.close(); // Close the file

    std::cout << "Data processing complete." << std::endl << std::endl;
}

// Validate data format and count
bool validateData(const std::vector<std::string>& dataStrings, int expectedDataCount) {
    if (dataStrings.size() != expectedDataCount) {
        std::cerr << "Error: Data count mismatch. Expected " << expectedDataCount << ", but received " << dataStrings.size() << std::endl;
        return false;
    }

    // Validate each data entry format
    for (const auto& data : dataStrings) {
        std::istringstream ss(data);
        std::string token;
        int count = 0;
        while (std::getline(ss, token, ',')) { // Split the received raw data string
            count++;
            // Check if each data item matches the expected format
            if (count == 1 || count == 2 || count == 3) {
                try {
                    std::stof(token);  // Check if it's a floating-point number
                } catch (const std::exception&) {
                    std::cerr << "Error: Invalid data format!" << count << ": " << token << std::endl;
                    return false;
                }
            } else if (count == 4) {
                try {
                    std::stol(token);  // Check if the timestamp is an integer
                } catch (const std::exception&) {
                    std::cerr << "Error: Invalid timestamp format: " << token << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}

// Server function: Receive data from the client, validate, store, and process
void receiveDataFromClient(int newSock, std::chrono::high_resolution_clock::time_point start, int processCount) {

    std::vector<std::string> dataStrings;  // To store a batch of received data
    char buffer[1024];  // Temporary buffer for receiving a single set of data

    int expectedDataCount = 10;  // Expected number of data entries per batch

    bool clientDisconnected = false; // Track if the client is disconnected

    while (true) {

        while(dataStrings.size() < 10){
            memset(buffer, 0, sizeof(buffer)); // Reset the temporary buffer before receiving each new entry

            // Receive data
            int bytesReceived = recv(newSock, buffer, sizeof(buffer), 0);
            if (bytesReceived == 0) {
                std::cout << "Client has closed the connection." << std::endl;
                std::cout << "Waiting for connection..." << std::endl;
                dataStrings.clear();
                clientDisconnected = true; // Mark client as disconnected
                break;
            } else if (bytesReceived < 0) {
                std::cerr << "Error: " << strerror(errno) << " (" << errno << ")" << std::endl;
                std::cout << "Waiting for connection..." << std::endl;
                dataStrings.clear();
                clientDisconnected = true;
                break;
            }

            std::string data(buffer); 
            dataStrings.push_back(data);

            std::cout << dataStrings.size() << " set of data has been received" << std::endl; // Print the number of received data sets
            if (dataStrings.size() == 10) std::cout << "This batch of data has been received" << std::endl << std::endl;
        }

        if (clientDisconnected) break;  // Exit the outermost loop

        // Validate data
        if (validateData(dataStrings, expectedDataCount)) {
            
            // Data validation passed, store data in local memory (here it is simply printed)
            std::cout << "Data received and validated. Storing data..." << std::endl << std::endl;

            for (const auto& data : dataStrings) {
                std::cout << "Received data: " << data << std::endl;
                std::cout << std::endl;
            }

            // Process data
            std::vector<std::string> dataStringsForProcess;
            for (const auto& data : dataStrings){
                dataStringsForProcess.push_back(data);
            }

            auto end = std::chrono::high_resolution_clock::now(); // Get the current time
            std::chrono::duration<double> elapsed = end - start; // Calculate elapsed time

            if (elapsed.count() > 60*processCount){ // Process data every minute
                std::cout << "Data processing..." << std::endl;
                processData(dataStringsForProcess);
                dataStringsForProcess.clear(); // Clear the processed data strings, preparing for the next batch
                processCount++;
            }

        } else {
            // Validation failed, send an error message to the client and ask for a resend
            std::string errorMessage = "Error: Invalid data format or data count mismatch. Please resend data.";
            send(newSock, errorMessage.c_str(), errorMessage.length(), 0);
        }

        // Clear received data strings and prepare for the next batch of data
        dataStrings.clear();

    }

    close(newSock);
}

// Server main function: Listen and accept client connections
void startServer(const std::string& serverIp, const int& serverPort) {

    auto start = std::chrono::high_resolution_clock::now();  // Record the program start time
    int processCount = 1; // Track the number of data processing cycles

    int serverSock;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error: Socket creation failed!" << std::endl;
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIp.c_str());
    serverAddr.sin_port = htons(serverPort);

    // Bind socket
    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error: Socket binding failed!" << std::endl;
        return;
    }

    // Start listening
    if (listen(serverSock, 1) < 0) {
        std::cerr << "Error: Listen failed!" << std::endl;
        return;
    }

    std::cout << "Server is listening on " << serverIp << ":" << serverPort << std::endl;

    // Wait for and accept client connections
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        int newSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (newSock < 0) {
            std::cerr << "Error: Accept failed! Retrying..." << std::endl;
            continue;
        }

        std::cout << "Client connected!" << std::endl;
        
        // Receive data from the client
        receiveDataFromClient(newSock, start, processCount);
    }

    close(serverSock);
}

int main() {
    std::string serverIp = "127.0.0.1";  // Server IP address
    int serverPort = 12345;  // Server port number

    // Start the server, wait for client connection, and receive data
    startServer(serverIp, serverPort);

    return 0;
}
