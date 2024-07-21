#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

// Include custom headers
#include "libspm.h"
#include "globals.h"
#include "cutils.h"

// Function to retrieve a package from a data repository
/*
Accepts:
- struct package* i_pkg: A pointer to a package structure with package details.
- const char* out_path: The local path to save the downloaded package.

Returns:
- char*: A pointer to the package format or NULL if there's an error.
*/
char* get(struct package* i_pkg, const char* repo, const char* out_path)
{
    // Check if the package name is specified
    if (i_pkg->name == NULL)
    {
        msg(ERROR, "Package name not specified!");
        return NULL;
    }

    return load_from_repo(i_pkg->name, repo, out_path);
}

int get_repos(char** list)
{
    dbg(3, "checking for repos");
    DIR *d;
    struct dirent *dir;
    d = opendir(getenv("SOVIET_REPOS_DIR"));
    int count = 0;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (count > 512)
            {
                printf("Error : too many elements in list , reallocating\n");
                list = realloc(list,(count+512) * sizeof(char*));
            }
            list[count] = calloc(strlen(dir->d_name) + 1, sizeof(char));
            strcpy(list[count], dir->d_name);
            count++;
        }
    }

    for (int i = 0; i < count - 1; i++)
    {
        if (strcmp(list[i], ".") == 0 || strcmp(list[i], "..") == 0)
        {
            // Move the . string to the end
            char* temp = list[i];
            dbg(3, "Moving: %s", temp);
            for (int k = i; k < count - 1; k++)
            {
                list[k] = list[k + 1];
            }
            list[count - 1] = temp;
            i--;
            count--;
        }
    }

    closedir(d);

    dbg(3, "done checking for repos");
    return count;
}

// Function to synchronize the local repository with a remote repository
int repo_sync() {
    const char* repo_dir = getenv("SOVIET_REPOS_DIR");
    const char* repo_url = getenv("SOVIET_DEFAULT_REPO_URL");
    const char* submodule_name = getenv("SOVIET_DEFAULT_REPO");

    char cmd[1024];

    // Check if the repository directory exists
    if (access(repo_dir, F_OK) != 0) {
        // Create the repository directory if it doesn't exist
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", repo_dir);
        if (system(cmd) != 0) {
            printf("Failed to create directory %s\n", repo_dir);
            return 1;
        }
    }

    // Check if repo_dir is a git repository
    if (access(repo_dir, X_OK) == 0) {
        // Change directory to repo_dir
        if (chdir(repo_dir) != 0) {
            printf("Failed to change directory to %s\n", repo_dir);
            return 1;
        }

        // Check if it's a git repository
        if (system("git rev-parse --is-inside-work-tree >/dev/null 2>&1") != 0) {
            // Initialize a new Git repository
            if (system("git init") != 0) {
                printf("Failed to initialize git repository in %s\n", repo_dir);
                return 2;
            }
        }

        // Check if submodule exists
        snprintf(cmd, sizeof(cmd), "git submodule status %s | grep -qF ' %s '", repo_dir, submodule_name);
        if (system(cmd) != 0) {
            // Add the submodule
            snprintf(cmd, sizeof(cmd), "git submodule add %s %s", repo_url, submodule_name);
            if (system(cmd) != 0) {
                printf("Failed to add submodule %s\n", submodule_name);
                return 2;
            }
        }

        // Update submodules
        if (system("git submodule update --remote --init --recursive") != 0) {
            printf("Failed to update submodules in %s\n", repo_dir);
            return 3;
        }
    } else {
        printf("%s is not a directory or cannot be accessed.\n", repo_dir);
    }

    return 0;
}