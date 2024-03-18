#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_PATH_LENGTH 1024
#define OPEN_ERROR -1
#define READ_ERROR -2 // You can define appropriate error codes

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

int count_installed() {
    DIR *dirp;
    struct dirent *dp;
    int fileCount = 0;
    char *directory = "/var/cccp/data/spm";

    dirp = opendir(directory);
    if (dirp == NULL) {
        perror("Error opening directory");
        return OPEN_ERROR;
    }

    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            fileCount++;
        }
    }

    if (errno != 0) {
        perror("Error reading directory");
        closedir(dirp);
        return READ_ERROR;
    }

    closedir(dirp);
    return fileCount;
}


int list_installed() {
    char* path = "/var/cccp/data/spm";
    DIR *d;
    struct dirent *dir;
    int count = 0;
    d = opendir(path);

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) // Check if it's a regular file
                count++;
        }
        closedir(d);
    } else {
        printf("Error: Unable to open directory %s\n", path);
        return -1; // Return -1 to indicate an error
    }

    return count;
}