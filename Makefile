CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2

SRC = src/main.cpp
TARGET = syslens

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)