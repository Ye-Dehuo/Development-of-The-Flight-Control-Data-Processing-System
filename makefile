# Specify the compiler
CXX=g++

# Compilation options, including C++17 support and debugging information
CXXFLAGS=-std=c++17 -g

# Target programs
TARGETS=client server query

# Default rule
all: $(TARGETS)

# Compile each target separately
client: client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o client

server: server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o server

query: query.cpp
	$(CXX) $(CXXFLAGS) query.cpp -o query

# Clean up compiled files
clean:
	rm -f $(TARGETS)

