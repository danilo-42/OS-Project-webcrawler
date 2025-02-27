/**
 * webcrawler.c
 * 
 * A simplified multithreaded web crawler with word counting.
 * 
 * Compilation (with Makefile provided):
 *    make all
 * Execution:
 *    make run
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <ctype.h>      // For tolower()
 
 #define MAX_URLS 50
 #define MAX_URL_LENGTH 2048
 #define NUM_KEYWORDS 3
 
 const char* keywords[NUM_KEYWORDS] = { "data", "science", "algorithm" };
 
 typedef struct {
     int index;
     char url[MAX_URL_LENGTH];
 } ThreadArgs;
 
 /**
  * Reads URLs from the specified file and stores them in the provided array.
  * Returns the number of URLs read, or -1 on error.
  */
 static int read_urls(const char* filename, char urls[][MAX_URL_LENGTH]) {
     // TODO: Open file and read URLs into the array
     return 0; // Placeholder
 }
 
 /**
  * Callback function for libcurl to write downloaded data to a file.
  */
 static size_t write_data(void* ptr, size_t size, size_t nmemb, void* userdata) {
     // TODO: Write the fetched data into the file
     return 0; // Placeholder
 }
 
 /**
  * Downloads the HTML content of the given URL and saves it to a file.
  */
 static void download_page(const char* url, const char* filename) {
     // TODO: Use libcurl to fetch the web page and save it to a file
 }
 
 /**
  * Opens the specified file, reads its content, and counts occurrences of keywords.
  */
 static void count_keywords_in_file(const char* filename) {
     // TODO: Read file, process text, and count occurrences of keywords
 }
 
 /**
  * Function executed by each thread to fetch and process a URL.
  */
 void* thread_func(void* arg) {
     // TODO: Construct filename, download page, count keywords
     return NULL; // Placeholder
 }
 
 /**
  * Main function:
  *   - Initializes curl
  *   - Reads URLs from file
  *   - Creates threads for fetching URLs
  *   - Waits for threads to finish
  *   - Cleans up
  */
 int main(void) {
     // TODO: Implement the main logic to initialize, create threads, and process URLs
     return 0; // Placeholder
 }
 