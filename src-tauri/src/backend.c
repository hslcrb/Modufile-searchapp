#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

typedef void (*FileCallback)(const char* name, const char* path, void* context);

#ifdef _WIN32
void walk_dir(const char* path, FileCallback callback, void* context) {
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);

    WIN32_FIND_DATA find_data;
    HANDLE h_find = FindFirstFile(search_path, &find_data);

    if (h_find == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s\\%s", path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Check for system/hidden folders to skip (rudimentary)
             if (find_data.cFileName[0] == '$' || strcmp(find_data.cFileName, "System Volume Information") == 0) {
                 continue;
             }
            walk_dir(full_path, callback, context);
        } else {
            callback(find_data.cFileName, full_path, context);
        }
    } while (FindNextFile(h_find, &find_data));

    FindClose(h_find);
}
#else
void walk_dir(const char* path, FileCallback callback, void* context) {
    DIR* dir = opendir(path);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Simple filtering of system dirs
        if (strncmp(path, "/proc", 5) == 0 || 
            strncmp(path, "/sys", 4) == 0 || 
            strncmp(path, "/dev", 4) == 0 || 
            strncmp(path, "/run", 4) == 0) {
            continue;
        }

        #ifdef _DIRENT_HAVE_D_TYPE
        if (entry->d_type == DT_DIR) {
             walk_dir(full_path, callback, context);
        } else if (entry->d_type == DT_REG) {
             callback(entry->d_name, full_path, context);
        }
        #else
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                walk_dir(full_path, callback, context);
            } else if (S_ISREG(st.st_mode)) {
                callback(entry->d_name, full_path, context);
            }
        }
        #endif
    }
    closedir(dir);
}
#endif

void start_walk(FileCallback callback, void* context) {
#ifdef _WIN32
    walk_dir("C:", callback, context); // Start from C: for Windows simplicity
#else
    walk_dir("/", callback, context);
#endif
}
