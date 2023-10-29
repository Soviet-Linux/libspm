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

// Function to download a repository from a given URL
/*
Accepts:
- const char* url_path: The path to the resource on the repository.
- const char* file_path: The local file path to save the downloaded resource.

Returns:
- int: An integer indicating the result of the download operation.
  - 0: Download success.
  - 1: Download failure.
*/
int downloadRepo(const char* url_path, const char* file_path)
{
    char** REPOS;
    int REPO_COUNT = splita(strdup(getenv("SOVIET_REPOS")), ' ', &REPOS);

    // Iterate over repositories
    for (int i = 0; i < REPO_COUNT; i++)
    {
        // Get the URL for the current repository
        char* repo = REPOS[i];
        printf("REPOS[%d] is '%s'\n", i, repo);

        // Create the full URL by combining the repository URL and the provided path
        char* url = calloc(strlen(repo) + strlen(url_path) + 8, sizeof(char));
        sprintf(url, "%s/%s", repo, url_path);

        // Log a message about the download process
        msg(INFO, "Downloading %s", url);

        // Attempt to download the file
        if (downloadFile(url, file_path) == 0)
        {
            // Clean up and return success
            free(url);
            free(*REPOS);
            free(REPOS);
            return 0;
        }
        // Clean up URL memory
        free(url);
    }
    // Clean up repository memory
    free(*REPOS);
    free(REPOS);
    // Return failure
    return 1;
}

// Function to download a file from a given URL and save it to a specified path
/*
Accepts:
- const char* url: The URL of the file to download.
- const char* file_path: The local file path to save the downloaded file.

Returns:
- int: An integer indicating the result of the download operation.
  - 0: Download success.
  - -1: Download failure.
*/
int downloadFile(const char* url, const char* file_path)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;

    // Initialize a CURL session
    curl = curl_easy_init();
    dbg(3, "curl_easy_init() returned %p", curl);

    // Check if the initialization was successful
    if (!curl)
    {
        msg(ERROR, "curl_easy_init() failed");
        return -1;
    }

    // Open a file for writing (binary mode)
    fp = fopen(file_path, "wb");

    // Set the URL for the CURL session
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // Set the write callback function to NULL (we write to the file directly)
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    // Disable the internal CURL progress meter
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);

    // Perform the download
    res = curl_easy_perform(curl);

    // Cleanup and close the file
    curl_easy_cleanup(curl);
    fclose(fp);
    printf("\n");

    // Check if the download was successful
    if (res != CURLE_OK)
    {
        return -1;
    }

    // Return success
    return 0;
}

// Function to check if a resource is present in a repository
/*
Accepts:
- CURL* session: The CURL session for performing the repository check.

Returns:
- bool: A boolean indicating if the resource is found in the repository.
  - true: Resource found in the repository.
  - false: Resource not found in the repository.
*/
bool is_in_repo(CURL* session)
{
    CURLcode curl_code;
    curl_code = curl_easy_perform(session);
    long http_code = 0;
    curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE, &http_code);

    // Check if the HTTP response code is 200 and the download was not aborted
    if (http_code == 200 && curl_code != CURLE_ABORTED_BY_CALLBACK)
    {
        printf("found in repo\n");
        return true;
    }
    else
    {
        return false;
    }
}
