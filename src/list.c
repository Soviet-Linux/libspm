#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_PATH_LENGTH 1024
#define OPEN_ERROR -1
#define READ_ERROR -2

char **getAllFiles(const char *path, int *num_files) {
    DIR *dir;
    struct dirent *entry;
    struct stat stat_buf;

    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "Error opening directory %s: %s\n", path, strerror(errno));
        return NULL;
    }

    char **files_array = NULL;
    int file_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (stat(full_path, &stat_buf) == 0) {
            if (S_ISDIR(stat_buf.st_mode)) {
                int sub_files_count;
                char **sub_files = getAllFiles(full_path, &sub_files_count);
                if (sub_files != NULL) {
                    files_array = realloc(files_array, (file_count + sub_files_count) * sizeof(char *));
                    for (int i = 0; i < sub_files_count; i++) {
                        files_array[file_count++] = sub_files[i];
                    }
                    free(sub_files);
                }
            } else if (S_ISREG(stat_buf.st_mode)) {
                files_array = realloc(files_array, (file_count + 1) * sizeof(char *));
                files_array[file_count] = strdup(full_path);
                file_count++;
            }
        }
    }
    closedir(dir);

    if (num_files != NULL)
        *num_files = file_count;

    return files_array;
}

int count_installed() {
    const char *path = "/var/cccp/data/spm";
    int num_files;
    char **files_array = getAllFiles(path, &num_files);
    if (files_array != NULL) {
        return num_files;
    } else {
        printf("No files found.\n");
    }
    return 1;
}

int list_installed() {
    const char *path = "/var/cccp/data/spm";
    int num_files;
    char **files_array = getAllFiles(path, &num_files);
    if (files_array != NULL) {
        for (int i = 0; i < num_files; i++) {
            printf("%s\n", files_array[i]);
            free(files_array[i]);
        }
        free(files_array);
    } else {
        printf("No files found.\n");
    }
    return 0;
}
