# Development-of-The-Flight-Control-Data-Processing-System
## Overview
- This project aims to design and implement a simple flight control data processing system, which includes flight control data simulation and acquisition, data transmission network module setup, as well as data processing, storage, and querying functions<br><br>
- The project mainly consists of three .cpp source files and one Makefile, with the following functions:<br><br>
-  `client.cpp` ：As the client, it is responsible for data simulation generation, acquisition, and transmission, and includes simple query functions and a reconnection mechanism<br><br>
-  `server.cpp` ：As the server, it establishes a network connection with the client via TCP/IP, receives the data sent by the client, and performs corresponding verification, storage, and processing of the data (such as calculating statistical values, generating log files, etc.)<br><br>
-  `query.cpp` ：As the query terminal, it allows users to input query conditions through the command-line interface to search for relevant files and output their contents <br><br>
-  `makefile` ：Bind the GCC compiler and input the relevant compilation rules, enabling the project to perform related operations in a Linux environment using the make commands
## Data acquisition terminal
### 1. Data generation  
- The `generateRandomAngle()` function is used to generate random attitude data, with the attitude angles constrained within the range of -180° to 180°
### 2. Data structure
- The `struct FlightData` represents the attitude data structure, storing the data elements `pitch`, `roll`, `yaw`, and `timestamp`
### 3. Function interface
- The `getDataCount()` function is used to retrieve the amount of data that has been collected <br><br>
- The `getDataAtIndex()` function is used to retrieve the data element content at a specified index position
## Data transmission network module
### 1. Data acquisition and transmission
- The `collectData()` function is used for data acquisition, collecting 10 sets of data at a time, and storing the data in the buffer `dataBuffer`. It simulates a 100ms time interval using `sleep_for(std::chrono::milliseconds())` <br><br>
- The `startClient()` function, as the client function, is used to connect to the server and send dat <br><br>
- Using the `socket` connection mechanism, the socket type is set to `SOCK_STREAM` to establish a TCP connection. The server's IP address and port are set to `127.0.0.1` and `12345`, respectively <br><br>
- The source code for the client and server are named `client.cpp` and `server.cpp`, respectively
### 2. Data transmission and reconnection mechanism
- The dynamic array `dataToSend` is used to store the latest 10 sets of data. The data is then converted into a string structure and transmitted using the `send()` function <br><br>
- The `isConnected` and `retryCount` are used to record the client's connection status and the number of reconnection attempts, respectively. If the client fails to connect (e.g., initial connection failure or server disconnection), it will attempt to reconnect, with a maximum of 5 reconnection attempts
### 3. Data reception, verification, and storage
- The `startServer()` function, as the server function, is used to wait for client connections and receive data <br><br>
- After initializing the socket, the `bind()` function is used to bind the socket, followed by the `listen()` function to start listening for incoming connections. The server waits for a client connection using `accept()`. If the client disconnects, the server will continue waiting at `accept()` until the client successfully reconnects or a new client connects <br><br>
- The `receiveDataFromClient()` function receives data from the client using `recv()` and stores the data in the buffer `dataStrings` <br><br>
- The `validateData()` function is used to validate the received data. It checks if the amount of received data meets the standard using `expectedDataCount`, verifies if the attitude angle data is a floating-point number using `stof()`, and checks if the timestamp is a long integer using `stol()`. If the validation fails, an error message is sent to the client, requesting that the data be resent <br><br>
- After the data validation passes, the data will be stored in local memory (in this case, just simply printed them)
## Data processing, file storage, and querying
### 1. Data processing and file storage
- Using `std::chrono` to record the system's running time ensures that data is processed once every minute for the received data
- The `processData()` function is used to process the validated data. It converts the data into floating-point numbers using the `convertData()` function, generates a filename with the date (derived from the timestamp) using the `generateFileName()` function, and calculates the maximum value, minimum value, and average value of the data using `max()`, `min()`, and `size()`. It then creates, writes, and closes the log file using file-related commands
### 2. Data querying
- The source code for the query terminal is `query.cpp`, which uses `cin` to record the user's input for the start and end times of the file query <br><br>
- The `queryData()` function is used to read data for a specified time range. It generates the filenames for the start and end times using the `generateFileName()` function. Then, using `std::filesystem::directory_iterator`, it iterates through all the files in the directory, selecting the files within the specified time range, and stores the filenames that meet the conditions in `filesToRead`. After that, it reads the file data using `getline()` and outputs the content <br><br>
- By starting the query terminal, the user can input a 14-digit "start time" and "end time" for the query. The system will then find the files within the specified time range and output the relevant data from those files to the terminal
## Git and GCC
- Write a `Makefile` to define the compilation rules and files, allowing compilation of related files using the `make` command. The `make clean` command is used to clean up intermediate files and target files generated during compilation



  

