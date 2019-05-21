CXX = g++

HEADER_DIR = ../include

FPTREE_HEADER_DIR = $(HEADER_DIR)/fptree

CXXFLAGS = -std=c++11 -Wall -g

CFLAGS = -fPIC -std=c99 -O3 -msse4.2 -mpclmul -march=native -funroll-loops -Wstrict-overflow -Wstrict-aliasing -Wall -Wextra -pedantic -Wshadow

MAIN = ./bin/main

YCSB = ./bin/ycsb

LYCSB = ./bin/lycsb

BIN = bin

FPTREE = fptree.o

UTILITY = utility.o

PALLOCATOR = pallocator.o

CLHASH = clhash.o

ALL := $(LYCSB) $(YCSB) $(MAIN) $(FPTREE) $(UTILITY) $(PALLOCATOR) $(CLHASH)

#============================TODO: all below============================
BUILD_DIR = ./build
INCLUDE = -I $(HEADER_DIR)

$(MAIN) : main.cpp $(BUILD_DIR)/$(FPTREE) $(BUILD_DIR)/$(UTILITY) $(BUILD_DIR)/$(PALLOCATOR) $(BUILD_DIR)/$(CLHASH) | $(BIN)
	$(CXX) $(CXXFLAGS)  $(INCLUDE) $^ -o $@


$(BUILD_DIR)/$(FPTREE) : fptree.cpp $(HEADER_DIR)/fptree/fptree.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(BUILD_DIR)/$(UTILITY) : utility.cpp $(HEADER_DIR)/utility/utility.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(BUILD_DIR)/$(PALLOCATOR) : p_allocator.cpp $(HEADER_DIR)/utility.p_allocator.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(HEADER_DIR)/utility.p_allocator.h:

$(HEADER_DIR)/utility/utility.h:

$(HEADER_DIR)/fptree/fptree.h:


$(BUILD_DIR)/$(CLHASH) : clhash.c $(HEADER_DIR)/utility/clhash.h
	$(CC) $(CFLAGS) -c clhash.c -I $(HEADER_DIR) -g -o $@

$(BIN):
	mkdir -p bin
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

all : $(ALL)

clean :
	rm -rf *.o ./bin/* ../data/*

cleand : 
	rm -rf ../data/*

run:$(BUILD_DIR)/$(PALLOCATOR)
