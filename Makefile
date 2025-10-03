CXX ?= g++
CXXFLAGS ?= -std=c++23 -O2 -Wall -Wextra -Iinclude
DEBUGFLAGS ?= -std=c++23 -g -O0 -Wall -Wextra -Iinclude

SRCS := src/main.cpp src/fsmachine.cpp
OBJS := $(SRCS:.cpp=.o)
TARGET := FSMachine$(EXE)

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo Please read README.md before run! Usage:
	@echo ./FSMachine run ^<filepath^> ^<input^>
	@echo ./FSMachine refactor ^<filepath1^> ^<filepath2^>

%.o: %.cpp %.hpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@
