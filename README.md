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
- `collectData()` 函数用于进行数据采集，一次采集 10 组数据，并将数据储存在缓存区 `dataBuffer` 中，通过 `sleep_for(std::chrono::milliseconds())` 模拟 100ms 时间间隔 <br><br>
- `startClient()` 作为客户端函数，用于连接到服务器并发送数据 <br><br>
- 利用套接字socket连接机制，并将类型设置为 `SOCK_STREAM` ，使用TCP连接。服务器Ip地址与端口分别设定为 `127.0.0.1` 与 `12345` <br><br>
- 客户端、服务端源代码分别为 `client.cpp` 与 `server.cpp`
### 2. 数据发送与重连机制
- 利用动态数组 `dataToSend` 储存最新 10 组数据，并将数据转换为字符串结构，通过 `send()` 发送数据 <br><br>
- 利用 `isConnected` 与 `retryCount` 分别记录客户端连接状态与重连次数，客户端在连接失败后（如客户端首次连接失败，服务端断联等），可尝试重新连接，且重连次数不能超过 5 次
### 3. 数据接收、校验与储存
- `startServer()`作为服务端函数，用于等待客户端连接并接收数据 <br><br>
- 套接字初始化后，利用 `bind()` 完成套接字绑定，随后利用 `listen()` 开启监听，等待客户端连接，并通过 `accept()` 接受连接。若客户端断联，服务端将一直等待 `accept`，直至客户端重连成功或新的客户端连接 <br><br>
- `receiveDataFromClient()` 函数通过 `recv()` 从客户端接收数据，并将数据存储在缓存区 `dataStrings` 中 <br><br>
- `validateData()` 函数用于校验接收到的数据，通过 `expectedDataCount` 校验接收数据量是否满足标准，通过 `stof()` 校验姿态角数据是否为浮点数，通过 `stol()` 校验时间戳是否为长整型。校验不通过则向客户端发送错误信息并要求重新发送 <br><br>
- 数据校验通过后，存储数据到本地内存（这里只是简单打印）
## 数据处理、文件储存与查询
### 1. 数据处理与文件储存
- 通过 `std::chrono` 记录系统运行时间，保证每分钟对接收到的数据进行一次处理
- `processData()` 函数用于对校验后的数据进行处理，通过 `convertData()` 函数将数据转换为浮点数，通过 `generateFileName()` 函数生成带日期（日期由时间戳转化）的文件名，利用 `max()` 、 `min()` 与 `size()` 计算数据最大值、最小值与平均值，并通过 `flie` 相关指令创建、写入与关闭记录文件
### 2. 数据查询
- 查询端源代码为 `query.cpp` ，利用 `cin` 记录用户输入的文件查询的起始与结束时间 <br><br>
- `queryData()` 函数用于读取指定时间段的数据，通过 `generateFileName()` 函数生成起始和结束的文件名，通过 `std::filesystem::directory_iterator` 遍历目录中的所有文件并选择时间范围内的文件，并将满足条件文件名存储在 `filesToRead` 中，随后通过 `getline()` 读取文件数据并输出<br><br>
- 通过开启查询端，即可通过输入 14 位的“查询起始时间”与“查询结束时间”找到时间范围内的文件，并将文件的相关数据输出到终端
## Git与GCC应用
- 编写 `makefile` 文件，确定编译规则与文件，可通过 `make` 命令编译相关文件，`make clean` 命令清理编译生成的中间文件和目标文件



  

