# Makefile for the Multithreaded Web Crawler

# mysys compile: for windows: gcc -std=c11 -Wall -Wextra webcrawler.c -o webcrawler -lcurl -lws2_32 -lcrypt32 -lz

# Compiler flags
CC = gcc
CFLAGS = -std=c11 -pedantic -pthread -lcurl

# Output binary name
TARGET = webcrawler

all:
	$(CC) $(CFLAGS) webcrawler.c -o $(TARGET)

clean:
	rm -f $(TARGET) page*.html

run:
	./$(TARGET)
