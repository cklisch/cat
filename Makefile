#makefile for ElevatorLogic

CPP_FILES := $(wildcard *.cpp)
OBJ_FILES := $(CPP_FILES:.cpp=.o)
CC_FLAGS := -D__GXXEXPERIMENTAL_CXX0X__ -Wall -std=c++0x

cat: $(OBJ_FILES)
	g++ -o $@ $^

%.o: %.cpp
	g++ $(CC_FLAGS) -c -o $@ $<

all: cat
