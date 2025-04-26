# OS-Project-webcrawler

Group project for Operating Systems class: Multithreaded Web Crawler

Created by Daniel Nikiforov, Raj Chauhan, Ethan Druce  

---

# Simplified Multithreaded Web Crawler with Word Counting

**Overview**  
This project implements a multithreaded web crawler in C using `pthreads` and `libcurl`.  
It concurrently fetches multiple web pages and counts user-specified keywords in the downloaded HTML files.

---

# Features

- **Multithreading**: Fetch multiple pages concurrently using `pthreads`
- **HTML Downloading**: Saves each page as `page_X.html`
- **Dynamic User Input**: 
  - Enter custom keywords (2–5)
  - Select how many URLs to crawl (50–150)
  - Select number of threads to use (1–32)
- **Word Counting**: Counts keyword occurrences in the saved HTML files
- **Error Handling**: Robust handling of invalid URLs, network errors, file I/O issues
- **Performance Timer**: Reports total execution time at the end

---

# Implementation Steps

1. **Input**:  
   - Program reads URLs from `urls.txt`
   - User specifies keywords, URL limit, and number of threads at runtime

2. **Multithreading**:  
   - Uses a shared task queue protected by a mutex
   - Threads pull tasks (URLs) and download independently

3. **Download**:  
   - Each page is downloaded using `libcurl`
   - Saved as `page_0.html`, `page_1.html`, etc.

4. **Word Counting**:  
   - After downloading, each HTML file is scanned line-by-line
   - Counts keyword occurrences (case-insensitive)

5. **Output**:  
   - Total occurrences for each keyword are displayed in the console
   - Execution time is reported at the end

---

# Example Makefile Commands

```make
all:
	gcc -std=c11 -Wall -Wextra -pedantic webcrawler.c -o webcrawler.exe -lcurl -lpthread -lws2_32 -lcrypt32 -lz

run:
	./webcrawler.exe

clean:
	rm -f webcrawler.exe page*.html
