#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <ctype.h>

// Maximum number of URLs to process
#define MAX_URLS 150
#define MAX_URL_LENGTH 2048
#define NUM_KEYWORDS 3
#define THREAD_POOL_SIZE 8 // Number of threads in the pool

// Keywords to search for in HTML pages
const char *keywords[NUM_KEYWORDS] = {"data", "science", "algorithm"};

// Global counter for total keyword occurrences across all pages
int total_counts[NUM_KEYWORDS] = {0};

// Mutexes to protect shared data
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// Struct for passing URL + index as a task
typedef struct {
    int index;
    char url[MAX_URL_LENGTH];
} ThreadArgs;

// Task queue for the thread pool
ThreadArgs task_queue[MAX_URLS];
int task_count = 0;
int current_task = 0;

/**
 * Converts a string to lowercase in-place.
 * Used to normalize lines before keyword searching.
 */
void str_tolower(char *str) {
    for (; *str; ++str) {
        *str = tolower((unsigned char)*str);
    }
}

/**
 * Counts occurrences of a keyword in a line, case-insensitively.
 * Returns the number of matches found.
 */
int count_keyword_occurrences(const char *line, const char *keyword) {
    int count = 0;
    size_t klen = strlen(keyword);

    for (const char *ptr = line; *ptr;) {
        if (strncasecmp(ptr, keyword, klen) == 0) {
            count++;
            ptr += klen;
        } else {
            ptr++;
        }
    }
    return count;
}

/**
 * Reads a list of URLs from a file (one per line).
 * Populates the given 2D array of strings.
 * Returns the number of URLs read, or -1 on error.
 */
static int read_urls(const char* filename, char urls[][MAX_URL_LENGTH]) {
    FILE *urlFile = fopen(filename, "r");
    if (!urlFile) {
        perror("Error Opening File");
        return -1;
    }

    int i = 0;
    while (i < MAX_URLS && fgets(urls[i], MAX_URL_LENGTH, urlFile) != NULL) {
        urls[i][strcspn(urls[i], "\r\n")] = '\0'; // Strip newline
        i++;
    }
    fclose(urlFile);
    return i;
}

/**
 * libcurl callback function to write downloaded HTML directly to a file.
 */
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
    FILE *dataFile = (FILE *)userdata;
    return fwrite(ptr, size, nmemb, dataFile);
}

/**
 * Downloads a URL's HTML content and saves it to a file on disk.
 * Uses libcurl and follows redirects.
 */
static void download_page(const char *url, const char *filename) {
    CURL *curl = curl_easy_init();
    if (curl) {
        FILE *file = fopen(filename, "w");
        if (file) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                fprintf(stderr, "Failed to fetch URL: %s\n", curl_easy_strerror(res));
            } else {
                printf("Downloaded %s to %s\n", url, filename);
            }
            fclose(file);
        } else {
            fprintf(stderr, "Failed to open file %s for writing.\n", filename);
        }
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize curl.\n");
    }
}

/**
 * Opens an HTML file and processes it line-by-line,
 * counting keyword matches for each keyword.
 * Uses str_tolower and count_keyword_occurrences for efficiency.
 */
static void count_keywords_in_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file %s.\n", filename);
        return;
    }

    char buffer[4096]; // Process in 4KB chunks (streaming, memory-safe)
    while (fgets(buffer, sizeof(buffer), file)) {
        str_tolower(buffer);
        for (int i = 0; i < NUM_KEYWORDS; i++) {
            int count = count_keyword_occurrences(buffer, keywords[i]);
            if (count > 0) {
                pthread_mutex_lock(&count_mutex);
                total_counts[i] += count;
                pthread_mutex_unlock(&count_mutex);
            }
        }
    }
    fclose(file);
}

/**
 * Worker thread function for the thread pool.
 * Pulls one task at a time from the shared queue and processes it.
 */
void *thread_func(void *arg) {
    while (1) {
        pthread_mutex_lock(&queue_mutex);
        if (current_task >= task_count) {
            pthread_mutex_unlock(&queue_mutex);
            break; // No more tasks to process
        }
        ThreadArgs task = task_queue[current_task++];
        pthread_mutex_unlock(&queue_mutex);

        char filename[256];
        snprintf(filename, sizeof(filename), "page_%d.html", task.index);
        download_page(task.url, filename);
        count_keywords_in_file(filename);
    }
    return NULL;
}

/**
 * Main entry point:
 * - Initializes curl
 * - Loads URLs from file
 * - Fills a task queue
 * - Spawns a thread pool to process them
 * - Waits for all threads to finish
 * - Prints total keyword counts
 */
int main(void) {
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        fprintf(stderr, "Failed to initialize curl.\n");
        return EXIT_FAILURE;
    }

    char urls[MAX_URLS][MAX_URL_LENGTH];
    int num_urls = read_urls("urls.txt", urls);
    if (num_urls <= 0) {
        fprintf(stderr, "Failed to read URLs from file.\n");
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    // Fill the task queue
    for (int i = 0; i < num_urls; i++) {
        task_queue[i].index = i;
        strncpy(task_queue[i].url, urls[i], MAX_URL_LENGTH - 1);
        task_queue[i].url[MAX_URL_LENGTH - 1] = '\0';
    }
    task_count = num_urls;

    // Start worker threads
    pthread_t threads[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }

    // Join all threads
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(threads[i], NULL);
    }

    // Final keyword report
    printf("\n=== Total Keyword Counts Across All Pages ===\n");
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        printf("Keyword '%s': %d occurrences\n", keywords[i], total_counts[i]);
    }

    // Cleanup
    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&queue_mutex);
    curl_global_cleanup();
    return EXIT_SUCCESS;
}
