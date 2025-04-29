# Makefile for the Multithreaded Web Crawler

# Instructions for compiling on Windows (MSYS2), and linux systems (auto detection):
# 1. Navigate to project directory:
#    cd "/c/Users/user/Desktop/OS Project/OS-project-webcrawler"
# 2. Compile:
#    make
# 3. Run:
#    make run
# 4. Clean:
#    make clean

# Compiler
CC = gcc

# Base CFLAGS that apply to both OS
BASE_CFLAGS = -std=c11 -Wall -Wextra -pedantic

# Linker flags for Windows (MSYS2)
LDFLAGS_WINDOWS = -lcurl -lws2_32 -lcrypt32 -lz -lpthread

# Linker flags for Linux (using pkg-config for libcurl libs)
LDFLAGS_LINUX = $(shell pkg-config --libs libcurl) -lpthread

SRC = webcrawler.c

# Detect OS and set TARGET, CFLAGS, and LDFLAGS accordingly
ifeq ($(OS),Windows_NT)
    TARGET = webcrawler.exe
    # CFLAGS for Windows - just use base flags
    CFLAGS = $(BASE_CFLAGS)
    LDFLAGS = $(LDFLAGS_WINDOWS)
else
    TARGET = webcrawler
    # CFLAGS for Linux - add pkg-config output for curl includes
    CFLAGS = $(BASE_CFLAGS) $(shell pkg-config --cflags libcurl)
    LDFLAGS = $(LDFLAGS_LINUX)
endif

# Default build target
all: $(TARGET) # Depend on the target file

$(TARGET): $(SRC) # Rule to build the target from the source file
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Run the compiled program
run: $(TARGET) # Ensure the target is built before running
	./$(TARGET)

# Clean the build artifacts
clean:
	rm -f $(TARGET) page*.html
