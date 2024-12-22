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

