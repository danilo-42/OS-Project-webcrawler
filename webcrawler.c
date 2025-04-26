#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <ctype.h>

#define MAX_TOTAL_URLS 150
#define MAX_URL_LENGTH 2048
#define MAX_KEYWORDS 5
#define MAX_KEYWORD_LENGTH 64

// Dynamic variables
int total_counts[MAX_KEYWORDS] = {0};
char keywords[MAX_KEYWORDS][MAX_KEYWORD_LENGTH];
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

// Task structure
typedef struct {
    int index;
    char url[MAX_URL_LENGTH];
} ThreadArgs;

// Shared work queue
ThreadArgs task_queue[MAX_TOTAL_URLS];
int task_count = 0;
int current_task = 0;

// User chosen values
int num_keywords;
int max_urls;
int num_threads;

/**
 * Converts a string to lowercase in-place.
 */
void str_tolower(char *str) {
    for (; *str; ++str) {
        *str = tolower((unsigned char)*str);
    }
}

/**
 * Counts how many times a keyword appears in a line (case-insensitive).
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
 * Reads URLs from a file and populates the urls array.
 */
static int read_urls(const char *filename, char urls[][MAX_URL_LENGTH]) {
    FILE *urlFile = fopen(filename, "r");
    if (!urlFile) {
        perror("Error Opening File");
        return -1;
    }

    int i = 0;
    while (i < max_urls && fgets(urls[i], MAX_URL_LENGTH, urlFile) != NULL) {
        urls[i][strcspn(urls[i], "\r\n")] = '\0'; // Remove newline
        i++;
    }
    fclose(urlFile);
    return i;
}

/**
 * libcurl callback to write downloaded HTML to a file.
 */
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
    FILE *dataFile = (FILE *)userdata;
    return fwrite(ptr, size, nmemb, dataFile);
}

/**
 * Downloads the given URL and saves to a file.
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
                printf("Downloaded %s -> %s\n", url, filename);
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
 * Processes an HTML file line-by-line, counting keywords.
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
                pthread_mutex_lock(&count_mutex);
                total_counts[i] += count;
                pthread_mutex_unlock(&count_mutex);
            }
        }
    }
    fclose(file);
}

/**
 * Worker thread function: fetch and process one task at a time.
 */
void *thread_func(void *arg) {
    while (1) {
        pthread_mutex_lock(&queue_mutex);
        if (current_task >= task_count) {
            pthread_mutex_unlock(&queue_mutex);
            break;
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
 * Main Program:
 * - Prompts user for keywords, URL limit, thread count
 * - Loads URLs
 * - Sets up thread pool
 * - Processes tasks
 * - Prints final report
 */
int main(void) {
    // Step 1: Ask for number of keywords
    printf("Enter number of keywords to search for (2-5): ");
    scanf("%d", &num_keywords);
    if (num_keywords < 2 || num_keywords > 5) {
        printf("Invalid keyword count. Defaulting to 3.\n");
        num_keywords = 3;
    }

    // Step 2: Ask for the actual keywords
    for (int i = 0; i < num_keywords; i++) {
        printf("Enter keyword #%d (max 63 characters): ", i + 1);
        scanf("%63s", keywords[i]);
        str_tolower(keywords[i]);
    }

    // Step 3: Ask for maximum number of URLs
    printf("Enter maximum number of URLs to crawl (50-150): ");
    scanf("%d", &max_urls);
    if (max_urls < 50 || max_urls > 150) {
        printf("Invalid URL count. Defaulting to 50.\n");
        max_urls = 50;
    }

    // Step 4: Ask for number of threads
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

    // Step 6: Create threads
    pthread_t threads[32]; // max 32
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Step 7: Print final keyword report
    printf("\n=== Total Keyword Counts Across All Pages ===\n");
    for (int i = 0; i < num_keywords; i++) {
        printf("Keyword '%s': %d occurrences\n", keywords[i], total_counts[i]);
    }

    pthread_mutex_destroy(&count_mutex);
    pthread_mutex_destroy(&queue_mutex);
    curl_global_cleanup();
    return EXIT_SUCCESS;
}
