#include <stdio.h>
#include <curl/curl.h>
#include <pthread.h>

void *fetch(void *arg) {
    const char *url = (const char*)arg;
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            long status = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
            printf("Fetched %s - HTTP %ld\n", url, status);
        }
        curl_easy_cleanup(curl);
    }
    return NULL; //ignore this comment
}

int main(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    pthread_t tid;
    const char *testURL = "https://www.lucyoutreach.org";
    if(pthread_create(&tid, NULL, fetch, (void*)testURL) != 0) {
        perror("pthread_create");
        return 1;
    }
    pthread_join(tid, NULL);
    curl_global_cleanup();
    return 0;
}
