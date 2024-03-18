#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

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

    return load_from_repo(i_pkg, out_path);
}

int get_repos(char** repo_list)
{
    char cmd[MAX_PATH + 64];
    sprintf(cmd, "cd %s && ls", getenv("SOVIET_REPOS_DIR"));
    char* res = exec(cmd);
    return splita(res, ' ', &repo_list);
}

// Function to synchronize the local repository with a remote repository
void sync()
{
    // copy tht's syncing code here
}
