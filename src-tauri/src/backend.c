#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

// --- Data Structures ---

typedef struct {
    char* name;
    char* path;
} FileInfo;

typedef struct {
    FileInfo* files;
    size_t count;
    size_t capacity;
} FileDatabase;

static FileDatabase db = {0};

typedef struct {
    int score;
    FileInfo* file;
} SearchResult;

// --- Helper Functions ---

void free_db() {
    if (db.files) {
        for (size_t i = 0; i < db.count; i++) {
            free(db.files[i].name);
            free(db.files[i].path);
        }
        free(db.files);
        db.files = NULL;
    }
    db.count = 0;
    db.capacity = 0;
}

void add_file(const char* name, const char* path) {
    if (db.count >= db.capacity) {
        size_t new_capacity = db.capacity == 0 ? 10240 : db.capacity * 2;
        FileInfo* new_files = realloc(db.files, new_capacity * sizeof(FileInfo));
        if (!new_files) return; // Allocation failed
        db.files = new_files;
        db.capacity = new_capacity;
    }
    db.files[db.count].name = strdup(name);
    db.files[db.count].path = strdup(path);
    db.count++;
}

// --- Fuzzy Matcher ---
// Simple implementation: checks subsequence and rewards consecutive logic
// Returns score > 0 if match, 0 if no match
int fuzzy_score(const char* str, const char* pattern) {
    int score = 0;
    int run = 0;
    
    // Case insensitive comparison
    while (*str && *pattern) {
        if (tolower(*str) == tolower(*pattern)) {
            score += 10 + (run * 5);
            run++;
            pattern++;
        } else {
            run = 0;
            score -= 1;
        }
        str++;
    }
    
    if (*pattern != '\0') return 0; // Not a full match of pattern
    return score > 0 ? score : 1;
}

// --- Walker Logic ---

#ifdef _WIN32
void walk_dir(const char* path) {
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);

    WIN32_FIND_DATA find_data;
    HANDLE h_find = FindFirstFile(search_path, &find_data);

    if (h_find == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) continue;

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s\\%s", path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
             if (find_data.cFileName[0] == '$' || strcmp(find_data.cFileName, "System Volume Information") == 0) continue;
            walk_dir(full_path);
        } else {
            add_file(find_data.cFileName, full_path);
        }
    } while (FindNextFile(h_find, &find_data));

    FindClose(h_find);
}
#else
void walk_dir(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Simple filtering
        if (strncmp(path, "/proc", 5) == 0 || 
            strncmp(path, "/sys", 4) == 0 || 
            strncmp(path, "/dev", 4) == 0 || 
            strncmp(path, "/run", 4) == 0 ||
            strncmp(path, "/tmp", 4) == 0) {
            continue;
        }

        #ifdef _DIRENT_HAVE_D_TYPE
        if (entry->d_type == DT_DIR) {
             walk_dir(full_path);
        } else if (entry->d_type == DT_REG) {
             add_file(entry->d_name, full_path);
        }
        #else
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                walk_dir(full_path);
            } else if (S_ISREG(st.st_mode)) {
                add_file(entry->d_name, full_path);
            }
        }
        #endif
    }
    closedir(dir);
}
#endif

// --- Exported Functions ---

int refresh_index_c() {
    free_db();
#ifdef _WIN32
    walk_dir("C:");
#else
    walk_dir("/");
#endif
    return (int)db.count;
}

typedef void (*ResultCallback)(const char* name, const char* path, void* context);

int compare_results(const void* a, const void* b) {
    SearchResult* ra = (SearchResult*)a;
    SearchResult* rb = (SearchResult*)b;
    return rb->score - ra->score; // Descending
}

void search_c(const char* query, int smart_match, ResultCallback callback, void* context) {
    if (!db.files) return;
    if (strlen(query) == 0) {
        // Return first 100
        for (size_t i = 0; i < db.count && i < 100; i++) {
            callback(db.files[i].name, db.files[i].path, context);
        }
        return;
    }

    // Since we can't easily alloc on stack for all, we use a limited result buffer
    // or we scan. For a "Smart Match", we usually need to score all and sort.
    // For large DB, this might be slow in C without optimization, but definitely faster than FFI overhead overhead of passing ALL strings.
    
    // Limits
    #define MAX_RESULTS 200
    SearchResult results[MAX_RESULTS];
    int result_count = 0;
    int min_score_in_top = -99999;
    
    // For smart match, we want top N. Implementing a min-heap would be best, 
    // but for simplicity, we'll just fill buffer and simplistic replacement or just simple scan if N is small?
    // Actually, sorting 500k items is slow.
    // Let's allocation a temp results array? No, unsafe.
    
    // Optimization: Just linear scan filtering, then if smart match, assign score. 
    // If getting TOP results is hard C-side without complex structures:
    
    // Let's implement a dynamic array for potential matches, sort, then return top 200.
    // Assuming max matches won't be HUGE or we assume user types specific queries.
    
    // Allow up to 2000 matches for sorting.
    SearchResult* matches = malloc(sizeof(SearchResult) * 2000);
    int match_count = 0;
    
    for (size_t i = 0; i < db.count; i++) {
        int score = 0;
        if (smart_match) {
            score = fuzzy_score(db.files[i].name, query);
        } else {
            // Substring match
             if (strcasestr(db.files[i].name, query)) score = 100; // Found
        }
        
        if (score > 0) {
            if (match_count < 2000) {
                matches[match_count].file = &db.files[i];
                matches[match_count].score = score;
                match_count++;
            }
        }
    }
    
    qsort(matches, match_count, sizeof(SearchResult), compare_results);
    
    int limit = match_count > 200 ? 200 : match_count;
    for (int i = 0; i < limit; i++) {
        callback(matches[i].file->name, matches[i].file->path, context);
    }
    
    free(matches);
}
 
