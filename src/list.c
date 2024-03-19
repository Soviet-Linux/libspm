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

// Function to recursively retrieve all files in a directory and its subdirectories
char **getAllFiles(const char *path, int *num_files) {
    DIR *dir;
    struct dirent *entry;
    struct stat stat_buf;

    // Open the directory
    dir = opendir(path);
    if (dir == NULL) {
        // Print an error message if directory couldn't be opened
        fprintf(stderr, "Error opening directory %s: %s\n", path, strerror(errno));
        return NULL;
    }

    // Initialize variables
    char **files_array = NULL;
    int file_count = 0;

    // Loop through directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip '.' and '..' entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construct full path of the current entry
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Get information about the current entry
        if (stat(full_path, &stat_buf) == 0) {
            // If it's a directory, recursively call getAllFiles
            if (S_ISDIR(stat_buf.st_mode)) {
                int sub_files_count;
                char **sub_files = getAllFiles(full_path, &sub_files_count);
                if (sub_files != NULL) {
                    // Resize files_array and copy contents of sub_files into it
                    files_array = realloc(files_array, (file_count + sub_files_count) * sizeof(char *));
                    for (int i = 0; i < sub_files_count; i++) {
                        files_array[file_count++] = sub_files[i];
                    }
                    free(sub_files);
                }
            } else if (S_ISREG(stat_buf.st_mode)) {
                // If it's a regular file, add it to files_array
                files_array = realloc(files_array, (file_count + 1) * sizeof(char *));
                files_array[file_count] = strdup(full_path);
                file_count++;
            }
        }
    }
    // Close the directory
    closedir(dir);

    // Update num_files if it's not NULL
    if (num_files != NULL)
        *num_files = file_count;

    // Return the array of file paths
    return files_array;
}

// Function to count the number of installed packages
int count_installed() {
    const char *path = "/var/cccp/data/spm";
    int num_files;
    char **files_array = getAllFiles(path, &num_files);
    if (files_array != NULL) {
        // Return the number of files
        return num_files;
    } else {
        // If no files found, print a message
        printf("No files found.\n");
    }
    return 1;
}

// Function to list all installed packages
int list_installed() {
    const char *path = "/var/cccp/data/spm";
    int num_files;
    char **files_array = getAllFiles(path, &num_files);
    if (files_array != NULL) {
        // Print the total number of files found
        printf("Total files found: %d\n", num_files);
        // Print each file path
        for (int i = 0; i < num_files; i++) {
            printf("%s\n", files_array[i]);
            // Free each file path string
            free(files_array[i]);
        }
        // Free the array of file paths
        free(files_array);
    } else {
        // If no files found, print a message
        printf("No files found.\n");
    }
    return 0;
}

// Function to search for a term in installed files
int search(char *term) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir("/var/cccp/data/repos"); // Open the current directory. Change "." to the directory path you want to search in.
    if (dir == NULL) {
        perror("Error opening directory");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, term) == 0) { // Compare the filename
            printf("Package not found: %s\n", entry->d_name);
            closedir(dir);
            return 0;
        }
    }

    printf("Package: %s\n", term);
    closedir(dir);
    return 0;
}
