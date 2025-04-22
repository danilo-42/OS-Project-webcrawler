# Makefile for the Multithreaded Web Crawler

# instructions to compile for windows run these commands in mysys terminal: 

# navigate to directory: cd "/c/Users/yourusername/Desktop/OS Project/OS-Project-webcrawler"
# run the following command to compile the code:
# gcc -std=c11 -Wall -Wextra webcrawler.c -o webcrawler -lcurl -lws2_32 -lcrypt32 -lz
# make run
# when finished: 
# make clean


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
