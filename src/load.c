#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
char* load_from_repo(const char* in, const char* in_repo, const char* file_path)
{
    // Try to find package in repos
    dbg(3, "loading %s from %s", in, in_repo);

    const char* path = calloc(MAX_PATH, 1);
    sprintf(path, "%s/%s", getenv("SOVIET_REPOS_DIR"), in_repo);

    int count;
    char **found = getAllFiles(path, path, &count);
    char* pkg = calloc(MAX_PATH + strlen(getenv("SOVIET_DEFAULT_FORMAT")) + 1, sizeof(char));
    if(!strstr(in, ".ecmp"))
    {
        sprintf(pkg, "%s.%s", in, getenv("SOVIET_DEFAULT_FORMAT"));
    }
    else
    {
        pkg = strdup(in);
    }

    if (found != NULL)
    {
        // Print each file path
        for (int i = 0; i < count; i++) 
        {

            // This will break if the files are not separated into repos
            // But it doesnt cause a crash, just a visual bug
            // I think
            dbg(3, "checking %s", found[i]);
            char* temp = strdup(found[i]);
            char* temp_path = strdup(found[i]);

            char* tok = strtok(temp, "/");
            char* repo = strdup(tok);
            char* package = NULL;
            while(tok != NULL )
            {
                tok = strtok(NULL, "/");
                if(tok != NULL)
                {
                    if(strstr(tok, ".ecmp"))
                    {
                        package = strdup(tok);
                    }
                }
            }
            if(package == NULL)
            {
                dbg(3, "%s not .ecmp package, moving on", found[i]);
                // Move the file to the end
                char* tar = found[i];
                for (int k = i; k < count - 1; k++)
                {
                    found[k] = found[k + 1];
                }
                found[count - 1] = tar;
                count--;
                i--;
            }
            else
            {
                dbg(3, "Comparing %s and %s", package, pkg);

                if (strcmp(package, pkg) == 0)
                {
                    // Compare the filename
                    dbg(3, "Loading package %s from %s", temp_path, in_repo);

                    // Create the full PATH by combining the repository URL and the provided path
                    char* path = calloc(strlen(repo) + strlen(getenv("SOVIET_REPOS_DIR")) + MAX_PATH, sizeof(char));
                    sprintf(path, "%s/%s/%s", getenv("SOVIET_REPOS_DIR"), in_repo, temp_path);
                    dbg(3, "Loading package from path: %s", path);
                    // Log a message about the download process

                    // Attempt to load the file
                    if (loadFile(path, file_path) == 0)
                    {
                        // Clean up and return success
                        free(path);
                        free(found);
                        return repo;
                    }
                    // Clean up URL memory
                    free(path);
                    free(found);
                }

                // Free each file path string
                free(package);
                free(repo);
                free(temp);
            }
        }
    }
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
    char cmd[PATH_MAX*2 + 64];
    sprintf(cmd, "cp %s %s", path, file_path);

    // Check if the download was successful
    // lmao
    if (system(cmd) == -1)
    {
        return -1;
    }
    // Return success

    return 0;
}
