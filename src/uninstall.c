#include "stdio.h"
#include "unistd.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// Include the necessary headers for the class file and utility functions
#include  "libspm.h"
#include "cutils.h"

// remove a file or link or directory
int rmany(char* path);

// Function to uninstall packages
/*
Accepts:
- char* name: The name of the package to uninstall.

Returns:
- int: An integer indicating the result of the uninstallation.
  - 0: The package was successfully uninstalled.
  - -1: An error occurred during the uninstallation.

Description:
This function is used to uninstall packages. It relies on location data, which contains all the files that were installed by the program. This data is stored in a JSON array inside the package's SPM file in DATA_DIR. The function cycles through all the files in the JSON array and removes them from the system. It also removes the package's entry from the installed packages database.

Please avoid making changes to this code unless there's a critical bug or an important missing feature.
*/
int uninstall(char* name)
{
    // Get the SPM directory from the environment variables
    char* SPM_DIR = getenv("SOVIET_SPM_DIR");
    dbg(3, "SOVIET_SPM_DIR = %s", SPM_DIR);
    char* ROOT = getenv("SOVIET_ROOT");

    char** REPOS = calloc(512,sizeof(char*));
    int REPO_COUNT = get_repos(REPOS);
    char* dataSpmPath = calloc(MAX_PATH, sizeof(char));

    // add local repo
    REPOS[REPO_COUNT] = "local";
    REPO_COUNT++;

    for (int j = 0; j < REPO_COUNT; j++)
    {
        // Generate the path to the package's SPM file
        char tmpSpmPath[MAX_PATH];
        sprintf(tmpSpmPath, "%s/%s/%s.%s", (char*)getenv("SOVIET_SPM_DIR"),REPOS[j], name, getenv("SOVIET_DEFAULT_FORMAT"));

        // Verify if the package is installed
        msg(INFO, "Verifying if the package is installed at %s", tmpSpmPath);

        // Check if the SPM file exists
        if (access(tmpSpmPath, F_OK) == 0) {
            sprintf(dataSpmPath, "%s/%s/%s.%s", getenv("SOVIET_SPM_DIR"),REPOS[j], name, getenv("SOVIET_DEFAULT_FORMAT"));
            // Create a struct to store package information
            struct package r_pkg;

            // Open the package's SPM file and populate the r_pkg struct
            open_pkg(dataSpmPath, &r_pkg, NULL);

            dbg(3, "Found %d locations", r_pkg.locationsCount);
            // Remove all the files in the data["locations"]
            char loc_path[MAX_PATH];
            for (int i = 0; i < r_pkg.locationsCount; i++) {

                sprintf(loc_path, "%s%s", ROOT, r_pkg.locations[i]);

                //dbg(3, "Removing %s", loc_path);                
                if (rmany(loc_path) != 0) {
                    msg(FATAL,"Failed to remove %s",loc_path);
                    perror("remove");
                }
            }
            // Remove the SPM file from DATA_DIR
            remove(dataSpmPath);
            return 0;
        }
    }
    msg(ERROR, "package not installed");
    return -1;
}


int rmany(char* path) {
    // check if its a symlink
    struct stat s;

    if (lstat(path, &s) == 0) {
        if (S_ISLNK(s.st_mode)) {
            // remove the symlink
             if (unlink(path) == 0) {
                return 0;
            } else {
                return -1;
            }
        }
        // check if its a directory
        if (S_ISDIR(s.st_mode)) {
            // remove the directory
            if (rmdir(path) == 0) {
                return 0;
            } else {
                msg(ERROR, "Error removing directory %s (Probably not empty)", path);
                return -1;
            }
        }
        // check if its a file
        if (S_ISREG(s.st_mode)) {
            // remove the file
            if (remove(path) == 0) {
                return 0;
            } else {
                return -1;
            }
        }
    } 
    return -1;
}