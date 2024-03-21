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
char* load_from_repo(const char* in, const char* file_path)
{
    // Try to find package in repos
    char cmd[PATH_MAX*2 + 64];
    sprintf(cmd, "cd %s/ && find . -name %s.ecmp", getenv("SOVIET_REPOS_DIR"),  in);
    char* res = exec(cmd);
    char** found;
    int count = splita(res, '\n', &found);

    switch (count)
    {
        case 0:
            msg(ERROR, "Failed to find %s", in);
            return NULL;
            break;
        case 1:
            if(strlen(found[0]) > 3)
            {
                found[0] += 2;
                char* repo = strtok(found[0], "/");
                char* package = strchr(found[0], '\0') + 1;

                dbg(3, "Loading package %s from %s", in, repo);

                // Create the full PATH by combining the repository URL and the provided path
                char* path = calloc(strlen(repo) + strlen(getenv("SOVIET_REPOS_DIR")) + MAX_PATH, sizeof(char));
                sprintf(path, "%s/%s/%s", getenv("SOVIET_REPOS_DIR"), repo, package);
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
            break;
        default:
            msg(INFO, "Multiple packages that match %s found, list %d results? y/N", in, count);

            char* str = calloc(2, sizeof(char));

            if(OVERWRITE_CHOISE != true)
            {
                char* res_2 = fgets(str, 2, stdin);

                if ( strchr(str, '\n') == NULL )
                {
                    while ((getchar()) != '\n');
                }

                int k = 0;

                while (str[k] != '\n' && str[k] != '\0')
                {
                    k++;
                }

                if (str[k] == '\n')
                {
                    str[k] = '\0';
                }
            }
            else
            {
                if(sizeof(USER_CHOISE[0]) == sizeof(str))
                {
                    sprintf(str, "%s", USER_CHOISE[0]);
                }
                else
                {
                    msg(FATAL, "something somwhere went wrong");
                }
            }
            if((strcmp(str, "Y") == 0 || strcmp(str, "y") == 0))
            {
                msg(INFO, "Listing all");

                for (int i = 0; i < count; i++)
                {
                    if(strlen(found[i]) > 3)
                    {
                        found[i] += 2;
                        char* repo = strtok(found[i], "/");
                        char* package = strchr(found[i], '\0') + 1;
                        msg(INFO, "%d. %s from %s", i, package, repo);

                    }
                }

                msg(INFO, "Choose which package to install");

                int int_ = calloc(2, sizeof(int));
                char* res_1 = scanf("%d", &int_);

                while ((getchar()) != '\n');

                if(int_ < count)
                {
                    dbg(3, "choise is %s", found[int_]);
                    char* repo = strtok(found[int_], "/");
                    char* package = strchr(found[int_], '\0') + 1;
                    msg(INFO, "Loading package %s from %s", package, repo);
                    // Create the full PATH by combining the repository URL and the provided path
                    char* path = calloc(strlen(repo) + strlen(getenv("SOVIET_REPOS_DIR")) + MAX_PATH, sizeof(char));
                    sprintf(path, "%s/%s/%s", getenv("SOVIET_REPOS_DIR"), repo, package);
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
                    free(path);
                    free(found);
                    // Clean up URL memory
                }
            }
            else
            {
                if(strlen(found[0]) > 3)
                {
                    found[0] += 2;
                    char* repo = strtok(found[0], "/");
                    char* package = strchr(found[0], '\0') + 1;
                    msg(INFO, "Loading package %s from %s", package, repo);
                    // Create the full PATH by combining the repository URL and the provided path
                    char* path = calloc(strlen(repo) + strlen(getenv("SOVIET_REPOS_DIR")) + MAX_PATH, sizeof(char));
                    sprintf(path, "%s/%s/%s", getenv("SOVIET_REPOS_DIR"), repo, package);
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
                    free(path);
                    free(found);
                    // Clean up URL memory
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
    if (system(cmd) == -1)
    {
        return -1;
    }
    // Return success

    return 0;
}
