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
     CURL* curl = curl_easy_init();

     if(curl)
     {
        //Open the file
        FILE* file = fopen(filename, "w");
        if(file)
        {
            //Sets URL, writes data into the file, follows redirects.
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            //Fetching.
            CURLcode res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                fprintf(stderr, "Failed to fetch URL: %s\n", curl_easy_strerror(res));
            } else{
                printf("Downloaded %s to %s\n", url, filename);
            }
            //Cleaning up.
            fclose(file);
        }else{
            fprintf(stderr, "Failed to open file %s for writing.\n", filename);
        }
        curl_easy_cleanup(curl);
     }else{
        fprintf(stderr, "Failed to initalize curl.\n");
     }
 }
 
 /**
  * Opens the specified file, reads its content, and counts occurrences of keywords.
  */
 static void count_keywords_in_file(const char* filename) {

     //Open file for reading. Error handling if it fails.
     FILE* file = fopen(filename, "r");
     if(!file)
     {
        fprintf(stderr, "Failed to open file %s.\n", filename);
        return;
     }

     //Allocating memory. I am using a fixed approach for simplicity, of 10 MB.
     size_t max_size = 10 * 1024 *1024; // 10 MB
     char* content = malloc(max_size);

     if(!content)
     {
        //If allocation fails, leave early.
         fprintf(stderr, "Failed to allocate memory for file content.\n");
         fclose(file);
         return;
     }
    
     size_t length = fread(content, 1, max_size - 1, file);
     content[length] = '\0'; // Null-terminate the string
     fclose(file);

     //Convert to lowercase.
     for(int i = 0; content[i]; i++)
     {content[i] = tolower((unsigned char)content[i]);}

     //Counting occurrences of keywords.
     for(int i = 0; i < NUM_KEYWORDS; i++)
     {
        const char* keyword = keywords[i];
        int count = 0;
        char* ptr = content;

        while((ptr = strstr(ptr, keyword)) != NULL)
        {
            count++;
            ptr += strlen(keyword);
        }
        printf("Keyword '%s' found %d times in %s\n", keyword, count, filename);
     }

     //Cleanup.
     free(content);
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
 