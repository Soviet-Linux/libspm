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
char* get(struct package* i_pkg, const char* out_path)
{
    // Check if the package name is specified
    if (i_pkg->name == NULL)
    {
        msg(ERROR, "Package name not specified!");
        return NULL;
    }

    return load_from_repo(i_pkg->name, out_path);
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
void sync()
{
    // copy tht's syncing code here
}
