#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <ctype.h>
#include <time.h>

// --- Constants and Definitions ---

#define MAX_TOTAL_URLS 150           // Maximum number of URLs supported
#define MAX_URL_LENGTH 2048          // Maximum length of a single URL
#define MAX_KEYWORDS 5               // Maximum number of keywords
#define MAX_KEYWORD_LENGTH 64        // Maximum length of a single keyword

// --- Global Variables ---

int total_counts[MAX_KEYWORDS] = {0}; // Array to hold the total count for each keyword
char keywords[MAX_KEYWORDS][MAX_KEYWORD_LENGTH]; // Array to store keywords entered by the user

// Mutex to safely update total_counts and task queue from multiple threads
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER; // Guards keyword counters
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER; // Guards task queue index

/**
 * Struct representing a single crawling task:
 * Holds an index (for filename) and the URL string itself.
 */
typedef struct {
    int index;                     // Index number of the URL (used for filenames)
    char url[MAX_URL_LENGTH];      // The actual URL string
} ThreadArgs;

// --- Shared Work Queue ---

ThreadArgs task_queue[MAX_TOTAL_URLS]; // Array representing all tasks (URLs)
int task_count = 0;                    // Total number of tasks loaded
int current_task = 0;                  // Next task index to fetch safely

// --- User Settings ---

int num_keywords;    // Number of keywords to search for
int max_urls;        // Maximum number of URLs to crawl
int num_threads;     // Number of threads to use

// --- Helper Functions ---

/**
 * Converts a string to lowercase in-place.
 * Used to normalize text for case-insensitive matching.
 */
void str_tolower(char *str) {
    for (; *str; ++str) {
        *str = tolower((unsigned char)*str);
    }
}

/**
 * Counts how many times a keyword appears in a line, case-insensitively.
 * Returns the number of matches found in that line.
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
 * Reads URLs from a given file ("urls.txt").
 * Fills the urls array up to the user-specified limit.
 */
static int read_urls(const char *filename, char urls[][MAX_URL_LENGTH]) {
    FILE *urlFile = fopen(filename, "r");
    if (!urlFile) {
        perror("Error Opening File");
        return -1;
    }

    int i = 0;
    while (i < max_urls && fgets(urls[i], MAX_URL_LENGTH, urlFile) != NULL) {
        urls[i][strcspn(urls[i], "\r\n")] = '\0'; // Remove trailing newline characters
        i++;
    }
    fclose(urlFile);
    return i;
}

/**
 * libcurl callback to write downloaded data directly into a file.
 */
static size_t write_to_file(void *ptr, size_t size, size_t nmemb, void *userdata) {
    FILE *dataFile = (FILE *)userdata;
    return fwrite(ptr, size, nmemb, dataFile);
}

/**
 * Downloads a single URL and saves it to a file on disk.
 * Follows redirects automatically.
 */
static void download_page(const char *url, const char *filename) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl.\n");
        return;
    }

    FILE *file = fopen(filename, "w");
    if (file) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow HTTP redirects automatically

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to fetch URL: %s --- %s\n", url, curl_easy_strerror(res));
        } else {
            printf("Downloaded %s -> %s\n", url, filename);
        }
        fclose(file);
    } else {
        fprintf(stderr, "Failed to open file %s for writing.\n", filename);
    }

    curl_easy_cleanup(curl);
}

/**
 * Processes a saved HTML file line-by-line,
 * Converts to lowercase, and counts keyword occurrences.
 */
static void count_keywords_in_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file %s.\n", filename);
        return;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), file)) {
        str_tolower(buffer);
        for (int i = 0; i < num_keywords; i++) {
            int count = count_keyword_occurrences(buffer, keywords[i]);
            if (count > 0) {
                pthread_mutex_lock(&count_mutex);     // Lock before modifying shared counter
                total_counts[i] += count;
                pthread_mutex_unlock(&count_mutex);   // Unlock after updating
            }
        }
    }
    fclose(file);
}

/**
 * Worker thread function:
 * Each thread repeatedly pulls the next available task safely,
 * downloads the page, saves it to file, and counts keyword occurrences.
 */
void *thread_func(void *arg) {
    (void)arg; // Unused thread argument

    while (1) {
        pthread_mutex_lock(&queue_mutex);  // Protect access to task queue

        if (current_task >= task_count) {
            pthread_mutex_unlock(&queue_mutex);
            //printf("Thread %lu finished all tasks and is exiting.\n", pthread_self());
            break; // No more tasks left
        }

        // Fetch the next available task and increment pointer
        ThreadArgs task = task_queue[current_task++];
        pthread_mutex_unlock(&queue_mutex);

        // Generate filename based on URL index
        char filename[256];
        snprintf(filename, sizeof(filename), "page_%d.html", task.index);

        download_page(task.url, filename);
        count_keywords_in_file(filename);
        //printf("Thread %lu finished processing page %d.\n", pthread_self(), task.index);
    }
    return NULL;
}

// --- Main method ---

/**
 * Prompts user for settings, loads URLs, spawns threads,
 * downloads and processes pages, and prints summary statistics.
 */
int main(void) {
    printf("=== Web Crawler ===\n");
    printf("This program will crawl a set of URLs and count keyword occurrences.\n");

    // Prompt user for number of keywords
    printf("Enter number of keywords to search for (2-5): ");
    scanf("%d", &num_keywords);
    if (num_keywords < 2 || num_keywords > 5) {
        printf("Invalid keyword count. Defaulting to 3.\n");
        num_keywords = 3;
    }

    // Ask for keywords
    for (int i = 0; i < num_keywords; i++) {
        printf("Enter keyword #%d (max 63 characters): ", i + 1);
        scanf("%63s", keywords[i]);
        str_tolower(keywords[i]);
    }

    // Ask for maximum number of URLs
    printf("Enter maximum number of URLs to crawl (50-150): ");
    scanf("%d", &max_urls);
    if (max_urls < 50 || max_urls > 150) {
        printf("Invalid URL count. Defaulting to 50.\n");
        max_urls = 50;
    }

    // Ask for number of threads
    printf("Enter number of threads to use (1-32): ");
    scanf("%d", &num_threads);
    if (num_threads < 1 || num_threads > 32) {
        printf("Invalid thread count. Defaulting to 8.\n");
        num_threads = 8;
    }

    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        fprintf(stderr, "Failed to initialize curl.\n");
        return EXIT_FAILURE;
    }

    // Step 5: Read URLs
    clock_t start_time = clock(); // Start timer
    char urls[MAX_TOTAL_URLS][MAX_URL_LENGTH];
    int num_urls = read_urls("urls.txt", urls);
    if (num_urls <= 0) {
        fprintf(stderr, "Failed to read URLs from file.\n");
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    // Fill task queue
    for (int i = 0; i < num_urls; i++) {
        task_queue[i].index = i;
        strncpy(task_queue[i].url, urls[i], MAX_URL_LENGTH - 1);
        task_queue[i].url[MAX_URL_LENGTH - 1] = '\0';
    }
    task_count = num_urls;

    // Create thread pool
    pthread_t threads[32];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }

    printf("\nAll threads created. Main thread is now waiting for workers to finish...\n");

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print final report
    printf("\n=== Total Keyword Counts Across All Pages ===\n");
    for (int i = 0; i < num_keywords; i++) {
        printf("Keyword '%s': %d occurrences\n", keywords[i], total_counts[i]);
    }

    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&queue_mutex);
    curl_global_cleanup();

    // Report total execution time
    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\nTotal execution time: %.2f seconds\n", time_taken);

    return EXIT_SUCCESS;
}
