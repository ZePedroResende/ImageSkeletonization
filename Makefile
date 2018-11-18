################################################################################
# Makefile for general code snippets
#
# by André Pereira (LIP-Minho)
################################################################################
SHELL = /bin/sh

BIN_NAME = skeleton_seq

CXX = g++
LD  = g++

detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
#CXX = icpc
#LD  = icpc

#-fopenmp/-openmp for GNU/Intel
ifeq ($(detected_OS),Darwin)
	CXXFLAGS = -Xpreprocessor  -fopenmp
else
	CXXFLAGS = -lpthread -fopenmp
endif
CXXFLAGS += -O3 -Wall -Wextra -std=c++11 -Wno-unused-parameter -pedantic -Werror 
#CXXFLAGS += 

ifeq ($(DEBUG),yes)
	CXXFLAGS += -ggdb3
endif

ifeq ($(DYNAMIC),yes)
	CXXFLAGS += -DD_DYNAMIC
endif

ifeq ($(IRREGULAR),yes)
	CXXFLAGS += -DD_IRREGULAR
endif

################################################################################
# Control awesome stuff
################################################################################

SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(patsubst src/%.cpp,build/%.o,$(SRC))
DEPS = $(patsubst build/%.o,build/%.d,$(OBJ))
BIN = $(BIN_NAME)

vpath %.cpp $(SRC_DIR)

################################################################################
# Rules
################################################################################

.DEFAULT_GOAL = all

$(BUILD_DIR)/%.d: %.cpp
	$(CXX) -M $(CXXFLAGS) $(INCLUDES) $< -o $@ 

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@ 

$(BIN_DIR)/$(BIN_NAME): $(DEPS) $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJ) 

checkdirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

all: checkdirs $(BIN_DIR)/$(BIN_NAME)

clean:
	rm -f $(BUILD_DIR)/* $(BIN_DIR)/*
	rm images/out_*
