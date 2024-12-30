# 指定编译器
CXX=g++

# 编译选项，这里添加了 C++17 支持和调试信息
CXXFLAGS=-std=c++17 -g

# 目标程序
TARGETS=client server query

# 默认规则
all: $(TARGETS)

# 单独编译每个目标
client: client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o client

server: server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o server

query: query.cpp
	$(CXX) $(CXXFLAGS) query.cpp -o query

# 清理编译生成的文件
clean:
	rm -f $(TARGETS)

