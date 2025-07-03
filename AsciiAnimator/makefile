# Makefile for App using GameEngine

CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I../GameEngine/src
LDFLAGS := ../GameEngine/bin/libgameengine.a -lform -lmenu -lncursesw

# Directories
SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin
TARGET := $(BIN_DIR)/main

# Sources and objects
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Stamp file for GameEngine builds
GAMEENGINE_STAMP := ../GameEngine/.build_stamp

# Default target
all: build_gameengine $(TARGET)

# GameEngine build rule
build_gameengine:
	@echo "Building GameEngine..."
	@$(MAKE) -C ../GameEngine
	@touch $(GAMEENGINE_STAMP)
	@# Check if any app object is older than engine build; if so, clean
	@for obj in $(OBJS); do \
	  if [ ! -f $$obj ] || [ $(GAMEENGINE_STAMP) -nt $$obj ]; then \
	    echo "Cleaning app due to updated GameEngine..."; \
	    $(MAKE) clean; \
	    break; \
	  fi \
	done

# Link target
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile .cpp to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean application build
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Full rebuild
rebuild:
	@echo "Forcing full rebuild..."
	@$(MAKE) -C ../GameEngine clean
	@$(MAKE) clean
	@$(MAKE) all

.PHONY: all clean rebuild build_gameengine

# Build and run the application
run: all
	@echo "Running the application..."
	@./$(TARGET)