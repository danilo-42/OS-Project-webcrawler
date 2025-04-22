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
#include <ctype.h> // For tolower()

#define MAX_URLS 50
#define MAX_URL_LENGTH 2048
#define NUM_KEYWORDS 3

const char *keywords[NUM_KEYWORDS] = {"data", "science", "algorithm"};

typedef struct
{
    int index;
    char url[MAX_URL_LENGTH];
} ThreadArgs;

/**
 * Reads URLs from the specified file and stores them in the provided array.
 * Returns the number of URLs read, or -1 on error.
 */
static int read_urls(const char *filename, char urls[][MAX_URL_LENGTH]) //Ethan
{
    // TODO: Open file and read URLs into the array
    return 0; // Placeholder
}

/**
 * Callback function for libcurl to write downloaded data to a file.
 */
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata) //Ethan
{
    // TODO: Write the fetched data into the file
    return 0; // Placeholder
}

/**
 * Downloads the HTML content of the given URL and saves it to a file.
 */
static void download_page(const char *url, const char *filename) //Raj
{
    CURL *curl = curl_easy_init();

    if (curl)
    {
        // Open the file
        FILE *file = fopen(filename, "w");
        if (file)
        {
            // Sets URL, writes data into the file, follows redirects.
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

            // Fetching.
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
                fprintf(stderr, "Failed to fetch URL: %s\n", curl_easy_strerror(res));
            }
            else
            {
                printf("Downloaded %s to %s\n", url, filename);
            }
            // Cleaning up.
            fclose(file);
        }
        else
        {
            fprintf(stderr, "Failed to open file %s for writing.\n", filename);
        }
        curl_easy_cleanup(curl);
    }
    else
    {
        fprintf(stderr, "Failed to initalize curl.\n");
    }
}

/**
 * Opens the specified file, reads its content, and counts occurrences of keywords.
 */
static void count_keywords_in_file(const char *filename) //Raj
{

    // Open file for reading. Error handling if it fails.
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s.\n", filename);
        return;
    }

    // Allocating memory. I am using a fixed approach for simplicity, of 10 MB.
    size_t max_size = 10 * 1024 * 1024; // 10 MB
    char *content = malloc(max_size);

    if (!content)
    {
        // If allocation fails, leave early.
        fprintf(stderr, "Failed to allocate memory for file content.\n");
        fclose(file);
        return;
    }

    size_t length = fread(content, 1, max_size - 1, file);
    content[length] = '\0'; // Null-terminate the string
    fclose(file);

    // Convert to lowercase.
    for (int i = 0; content[i]; i++)
    {
        content[i] = tolower((unsigned char)content[i]);
    }

    // Counting occurrences of keywords.
    for (int i = 0; i < NUM_KEYWORDS; i++)
    {
        const char *keyword = keywords[i];
        int count = 0;
        char *ptr = content;

        while ((ptr = strstr(ptr, keyword)) != NULL)
        {
            count++;
            ptr += strlen(keyword);
        }
        printf("Keyword '%s' found %d times in %s\n", keyword, count, filename);
    }

    // Cleanup.
    free(content);
}

/**
 * Function executed by each thread to fetch and process a URL.
 * Downloads the HTML content, saves to a file, then counts keyword frequency.
 */
void *thread_func(void *arg) //Dan
{
    // Cast the argument to a pointer to ThreadArgs
    ThreadArgs *threadArgs = (ThreadArgs *)arg;

    // Construct a unique filename for this thread (e.g., page_0.html)
    char filename[256];
    snprintf(filename, sizeof(filename), "page_%d.html", threadArgs->index);

    // Download the page content and save it
    download_page(threadArgs->url, filename);

    // Analyze the file for keyword frequency
    count_keywords_in_file(filename);

    return NULL;
}

/**
 * Main function:
 *   - Initializes curl
 *   - Reads URLs from file
 *   - Creates threads for fetching URLs
 *   - Waits for threads to finish
 *   - Cleans up
 */
int main(void) //Dan
{
    // Initialize curl globally
    if (curl_global_init(CURL_GLOBAL_ALL) != 0)
    {
        fprintf(stderr, "Failed to initialize curl.\n");
        return EXIT_FAILURE;
    }

    // Array to store URLs
    char urls[MAX_URLS][MAX_URL_LENGTH];
    int num_urls = read_urls("urls.txt", urls);
    if (num_urls <= 0)
    {
        fprintf(stderr, "Failed to read URLs from file.\n");
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    // Thread handles and argument structs
    pthread_t threads[MAX_URLS];
    ThreadArgs threadArgs[MAX_URLS];

    // Launch a thread for each URL
    for (int i = 0; i < num_urls; i++)
    {
        threadArgs[i].index = i;
        strncpy(threadArgs[i].url, urls[i], MAX_URL_LENGTH - 1);
        threadArgs[i].url[MAX_URL_LENGTH - 1] = '\0'; // Null-terminate

        if (pthread_create(&threads[i], NULL, thread_func, &threadArgs[i]) != 0)
        {
            fprintf(stderr, "Failed to create thread for URL: %s\n", urls[i]);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_urls; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Global curl cleanup
    curl_global_cleanup();
    return EXIT_SUCCESS;
}
