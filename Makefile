# Makefile for the Multithreaded Web Crawler

# Instructions for compiling on Windows (MSYS2):
# 1. Navigate to project directory:
#    cd "/c/Users/username/Desktop/OS Project/OS-Project-webcrawler"
# 2. Compile:
#    gcc -std=c11 -Wall -Wextra webcrawler.c -o webcrawler -lcurl -lws2_32 -lcrypt32 -lz -lpthread
# 3. Run:
#    ./webcrawler.exe
# 4. Clean:
#    make clean

# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
LDFLAGS_WINDOWS = -lcurl -lws2_32 -lcrypt32 -lz -lpthread
LDFLAGS_LINUX = -lcurl -lpthread
SRC = webcrawler.c

# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET = webcrawler.exe
    LDFLAGS = $(LDFLAGS_WINDOWS)
else
    TARGET = webcrawler
    LDFLAGS = $(LDFLAGS_LINUX)
endif

# Default build target
all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Run the compiled program
run: all
	./$(TARGET)

# Clean the build artifacts
clean:
	rm -f $(TARGET) page*.html
