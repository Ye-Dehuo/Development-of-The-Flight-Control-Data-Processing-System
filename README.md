# Development-of-The-Flight-Control-Data-Processing-System
## 概述
- 本项目旨在设计并实现一个简单的飞控数据处理系统，包含飞控数据模拟采集、数据传输网络模块搭建以及数据处理与存储
- 项目主要包含文件为 3 个源代码与 1 个 `makefile` 文件，其功能分别如下
-  `client.cpp` ：作为客户端，负责数据模拟生成、采集与发送，并含有简单的查询类函数接口与重连机制
-  `server.cpp` ：作为服务端，通过 TCP/IP 与客户端建立网络连接，可接收客户端发送的数据，并对数据进行相应校验、存储与处理（计算统计值、生成记录文件等）
-  `query.cpp` ：作为查询端，通过命令行界面，允许用户通过键入查询条件进行相关文件的检索与内容输出
## 数据采集端
### 1. 数据生成  
- `generateRandomAngle()` 函数用于生成随机姿态数据，并将姿态角限制在 -180° 至 180° <br>
### 2. 数据结构
- `struct FlightData` 表示姿态数据结构，存储 `pitch` `roll` `yaw` 与`timestamp` 数据元素 <br>
### 3. 函数接口
- `getDataCount()` 函数用于获取已采集数据量 <br><br>
- `getDataAtIndex()` 函数用于获取指定索引位置的数据元素内容
## 数据传输网络模块
### 1. 数据采集与发送
- `collectData()` 函数用于进行数据采集，一次采集10组数据，并将数据储存在缓存区 `dataBuffer` 中，通过 `sleep_for(std::chrono::milliseconds())` 模拟100ms时间间隔 <br>
- `startClient()` 作为客户端函数，用于连接到服务器并发送数据 <br>
- 利用套接字socket连接机制，并将类型设置为 `SOCK_STREAM` ，使用TCP连接。服务器Ip地址与端口分别设定为 `127.0.0.1` 与 `12345` <br>
- 客户端、服务端源代码分别为 `client.cpp` 与 `server.cpp`
### 2. 数据发送与重连机制
- 利用动态数组 `dataToSend` 储存最新10组数据，并将数据转换为字符串结构，通过 `send()` 发送数据 <br>
- 利用 `retryCount` 记录客户端重连次数，确保客户端在连接失败后，可尝试重新连接，且重连次数不能超过 5 次 <br>
### 3. 数据接收、校验与储存
- `startServer()`作为服务端函数，用于等待客户端连接并接收数据 <br>
- 套接字初始化后，利用 `bind()` 完成套接字绑定，随后利用 `listen()` 开启监听，等待客户端连接，并通过 `accept()` 接受连接 <br>
- `receiveDataFromClient()` 函数通过 `recv()` 从客户端接收数据，并将数据存储在缓存区 `dataStrings` 中 <br>
- `validateData()` 函数用于校验接收到的数据，通过 `expectedDataCount` 校验接收数据量是否满足标准，通过 `stof()` 校验姿态角数据是否为浮点数，通过 `stol()` 校验时间戳是否为长整型。校验不通过则向客户端发送错误信息并要求重新发送 <br>
- 数据校验通过后，存储数据到本地内存（这里只是简单打印）
## 数据处理与存储端
### 1. 数据处理与文件储存
- `processData()` 函数用于对校验后的数据进行处理，通过 `convertData()` 函数将数据转换为浮点数，通过 `generateFileName()` 函数生成带日期（日期由时间戳转化）的文件名，利用 `max()` 、 `min()` 与 `size()` 计算数据最大值、最小值与平均值，并通过 `flie` 相关指令创建、写入与关闭记录文件 <br>
- 查询端源代码为 `query.cpp` ，利用 `cin` 记录用户输入的文件查询的起始与结束时间 <br>
- `queryData()` 函数用于读取指定时间段的数据，通过 `generateFileName()` 函数生成起始和结束的文件名，通过 `std::filesystem::directory_iterator` 遍历目录中的所有文件并选择时间范围内的文件，并将满足条件文件名存储在 `filesToRead` 中，随后通过 `getline()` 读取文件数据并输出
- 通过开启查询端，即可通过输入“查询的起始时间”与“查询的结束时间”找到时间范围内的文件，并将文件数据输出到终端
## Git与GCC应用
- 编写 `makefile` 文件，确定编译规则与文件，可通过 `make` 命令编译相关文件，`make clean` 命令清理编译生成的中间文件和目标文件



  

