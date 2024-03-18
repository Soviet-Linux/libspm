#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#include <curl/curl.h>

// Include necessary headers

// class stuff
#include "libspm.h"
#include "cutils.h"

// Additional custom header includes

// Function to get a file from a local repository
/*
Accepts:
- const char* file_path: The local file path to save the downloaded resource.

Returns:
- int: An integer indicating the result of the download operation.
  - 0: get success.
  - 1: get failure.
*/
char* load_from_repo(const char* in, const char* file_path)
{
    //TODO write a way to chose if multiple files are found
    dbg(3, "loading repositories");
    char** REPOS = calloc(512,sizeof(char));
    int REPO_COUNT = get_repos(REPOS);
    dbg(3, "Got %d repos", REPO_COUNT);

    for(int p = 0; p < REPO_COUNT; p++)
    {
        dbg(3, "%s", REPOS[p]);
    }

    // Iterate over repositories
    for (int i = 0; i < REPO_COUNT; i++)
    {
        // Get the path for the current repository
        char* repo = NULL;
        repo = calloc(strlen(REPOS[i] + 1), sizeof(char));
        strcpy(repo, REPOS[i]);

        dbg(3, "Loading package %s from %s", in, repo);

        // Try to find package in repo
        char cmd[PATH_MAX*2 + 64];
        sprintf(cmd, "cd %s/%s && find . -name %s.ecmp", getenv("SOVIET_REPOS_DIR"), repo,  in);
        char* res = exec(cmd);
        char** found;
        int count = splita(res, '\n', &found);

        if(count == 0)
        {
            msg(ERROR, "Failed to find %s", in);
            return NULL;
        }
        for(int j = 0; j < count; j++)
        {
            //TODO loop over all of the options that share the same name and ask the user to choose

            // Create the full PATH by combining the repository URL and the provided path
            char* path = calloc(strlen(repo) + strlen(getenv("SOVIET_REPOS_DIR")) + 8, sizeof(char));
            sprintf(path, "%s/%s/%s", getenv("SOVIET_REPOS_DIR"), repo, found[j]);
            dbg(3, "Loading package from path: %s", path);
            // Log a message about the download process

            // Attempt to load the file
            if (loadFile(path, file_path) == 0)
            {
                // Clean up and return success
                free(path);
                //free(*REPOS);
                free(REPOS);
                return repo;
            }
            // Clean up URL memory
            free(path);
        }
    }
    // Clean up repository memory
    //free(*REPOS);
    free(REPOS);
    // Return failure
    return NULL;
}

// Function to load a file from a given repo and save it to a specified path
/*
Accepts:
- const char* path: The path of the file to download.
- const char* file_path: The local file path to save the downloaded file.

Returns:
- int: An integer indicating the result of the download operation.
  - 0: Download success.
  - -1: Download failure.
*/
int loadFile(const char* path, const char* file_path)
{
    char cmd[PATH_MAX + 64];
    sprintf(cmd, "cp %s %s", path, file_path);
    exec(cmd);

    // Check if the download was successful
    if (!exec(cmd))
    {
        return -1;
    }
    // Return success
    return 0;
}
