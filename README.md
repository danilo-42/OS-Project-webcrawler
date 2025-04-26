# OS-Project-webcrawler
Group project for OS class: Multithreaded webcrawler

Created by Daniel Nikiforov, Raj Chauhan, Ethan Druce  

# Simplified Multithreaded Web Crawler with Word Counting

**Overview**  
This project uses `pthreads` to fetch multiple web pages concurrently (via `libcurl`) and counts the occurrences of important words in the downloaded HTML content.

**Features**  
- **Multithreading**: Several URLs run in their own threads concurrently.
- **HTML Fetching**: Saves each page as `page1.html`, `page2.html`, etc.  
- **Word Counting**: Counts predefined keywords in each HTML file  
- **Error Handling**: Handles invalid URLs, network failures, and file I/O issues  

**Implementation Steps**  
1. **Input**: Read URLs from `urls.txt`  
2. **Threading**: Use `pthread_create` for each URL  
3. **Download**: Fetch HTML with `libcurl` and save as `pageX.html`  
4. **Word Counting**: Parse downloaded files for keyword occurrences  
5. **Output**: Print logs and word counts to the console  

**Example Makefile**  
all: gcc -std=c11 -pedantic -pthread -lcurl crawler.c -o crawler

clean: rm -f crawler page*.html

run: ./crawler


**How to Use**  
1. Put URLs in `urls.txt` (one per line)  
2. Run `make all` to compile  
3. Run `make run` to execute the crawler  
4. Run `make clean` to clean up the html pages after

**File Structure**  

**File Structure**  
. ├── crawler.c // Main source code ├── urls.txt // List of URLs ├── Makefile // Build instructions └── page*.html // Downloaded HTML files
