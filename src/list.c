#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

// Include necessary headers
#include "libspm.h"
#include "cutils.h"

#define MAX_PATH_LENGTH 1024
#define OPEN_ERROR -1
#define READ_ERROR -2

// Function to recursively retrieve all files in a directory and its subdirectories
char **get_all_files(const char* root, const char *path, int *num_files) {
    DIR *dir;
    struct dirent *entry;
    struct stat stat_buf;
    char* origin = strdup(path);
    // Open the directory
    dir = opendir(path);
    if (dir == NULL) {
        // Print an error message if directory couldn't be opened
        fprintf(stderr, "Error opening directory %s: %s\n", path, strerror(errno));
        free(origin);
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
            // If it's a directory, recursively call get_all_files
            if (S_ISDIR(stat_buf.st_mode)) {
                struct stat dir_stat_buf;
                if (lstat(full_path, &dir_stat_buf) == 0) {
                    // Check if a directory is a symlink (this can probably be optimized out)
                    if (!S_ISLNK(dir_stat_buf.st_mode)) {
                        // If it isn't - treat it as a directory
                        int sub_files_count;
                        char **sub_files = get_all_files(root, full_path, &sub_files_count);
                        if (sub_files != NULL) {
                            // Resize files_array and copy contents of sub_files into it
                            files_array = realloc(files_array, (file_count + sub_files_count) * sizeof(char *));
                            for (int i = 0; i < sub_files_count; i++) {
                                files_array[file_count++] = sub_files[i];
                            }
                            free(sub_files);
                        }
                    }
                    else {
                        // If it is - treat it as a file
                        files_array = realloc(files_array, (file_count + 1) * sizeof(char *));
                        files_array[file_count] = strdup(full_path + strlen(root) + 1);
                        file_count++;
                    }
                }
            } else if (S_ISREG(stat_buf.st_mode)) {
                // If it's a regular file, add it to files_array
                files_array = realloc(files_array, (file_count + 1) * sizeof(char *));
                files_array[file_count] = strdup(full_path + strlen(root) + 1);
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
    free(origin);

    return files_array;
}

// Function to count the number of installed packages
int count_installed() {
    const char *path = getenv("SOVIET_SPM_DIR");
    int num_files;
    char **files_array = get_all_files(path, path, &num_files);
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
    const char *path = getenv("SOVIET_SPM_DIR");
    int num_files;
    char **files_array = get_all_files(path, path, &num_files);
    if (files_array != NULL) {
        // Print each file path
        for (int i = 0; i < num_files; i++) {

            // This will break if the files are not separated into repos
            // But it doesnt cause a crash, just a visual bug
            // I think
            char* repo = strtok(files_array[i], "/");
            char* package = strchr(files_array[i], '\0') + 1;
            printf("%s from %s\n", package, repo);

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
char ** search(char *term, int *num_results) {
    int found = 0;
    const char *path = getenv("SOVIET_REPOS_DIR");
    int num_files;
    char **files_array = get_all_files(path, path, &num_files);
    char **searched_array = NULL;

    if (files_array != NULL) {
        // Print each file path
        for (int i = 0; i < num_files; i++) 
        {

            // This will break if the files are not separated into repos
            // But it doesnt cause a crash, just a visual bug
            // I think
            char* repo = strtok(files_array[i], "/");
            char* package = strchr(files_array[i], '\0') + 1;

            char* package_name = calloc(strlen(package) + 2, sizeof(char));
            strcpy(package_name, package);

            while(strtok(package_name, "/"))
            {
                char* tmp = package_name;
                package_name = strchr(package_name, '\0') + 1;
                if(*package_name == '\0')
                {
                    package_name = tmp;
                    break;
                }
            }

            dbg(1, "repos is %s, package is %s, name is %s",repo, package, package_name);

            if (strstr(package_name, term) != 0)
            {
                // Compare the filename
                printf("Found package: %s in %s \n", package_name, repo);
                searched_array = realloc(searched_array, (found + 1) * sizeof(char *));
                // Stupid
                char* tmp = calloc(strlen(package_name) + strlen(repo) * 2, 1);
                sprintf(tmp, "%s>%s", package_name, repo);

                searched_array[found] = strdup(tmp);
                free(tmp);
                found++;
            }

            // Free each file path string
            free(files_array[i]);
        }
        // Free the array of file paths
        free(files_array);

        // Update num_results if it's not NULL
        if (num_results != NULL)
            *num_results = found;

        return searched_array;
    } else {
        // If no files found, print a message
        printf("All repositories are empty.\n");
        return NULL;
    }

    if(found == 0)
    {
        printf("Package not found: %s\n", term);
    }
    return 0;
}

