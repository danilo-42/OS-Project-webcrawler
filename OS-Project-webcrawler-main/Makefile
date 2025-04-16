# Makefile for the Multithreaded Web Crawler

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
