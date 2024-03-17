#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH_LENGTH 1024

char **getAllFiles(const char *path, int *num_files) {
    char **files_array = NULL;
    int file_count = 0;

    DIR *dir;
    struct dirent *entry;
    struct stat stat_buf;

    if (!(dir = opendir(path)))
        return NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char next_path[MAX_PATH_LENGTH];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(next_path, sizeof(next_path), "%s/%s", path, entry->d_name);
            getAllFiles(next_path, num_files);
        } else {
            char full_path[MAX_PATH_LENGTH];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            if (stat(full_path, &stat_buf) == 0 && S_ISREG(stat_buf.st_mode)) {
                files_array = (char **)realloc(files_array, (file_count + 1) * sizeof(char *));
                files_array[file_count] = (char *)malloc(MAX_PATH_LENGTH * sizeof(char));
                // Extract the last directory name from the path
                char *last_dir = strrchr(path, '/');
                if (last_dir != NULL)
                    last_dir++; // Move past the '/'
                else
                    last_dir = (char *)path; // No '/' found, use the path itself
                snprintf(files_array[file_count], MAX_PATH_LENGTH, "%s/%s", last_dir, entry->d_name);
                file_count++;
            }
        }
    }
    closedir(dir);

    if (num_files != NULL)
        *num_files = file_count;

    if (file_count == 0)
        return NULL;

    return files_array;
}

int count_installed(char *basePath) {
    int file_count = 0;
    DIR * dirp;
    struct dirent * entry;
    while ((entry = readdir(basePath)) != NULL) {
        if (entry->d_type == DT_REG) { /* If the entry is a regular file */
            file_count++;
        }
    }
    closedir(basePath);
}


int list_installed(const char *path) {
    int count = 0;
    DIR *dir;
    struct dirent *entry;

    // Open directory
    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return -1;
    }

    // Iterate through directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Ignore current directory and parent directory entries
        if (entry->d_type == DT_REG) {
            count++;
        }
    }

    closedir(dir);
    printf("%s", count);
    return 0;
}