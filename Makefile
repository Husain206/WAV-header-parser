CXX       := g++
CXXFLAGS  := -Wall -Wextra -Wpedantic -ftime-report
SRC       := $(wildcard main.cpp src/*.cpp)
OBJ       := $(SRC:.cpp=.o)
TARGET    := wav

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
