#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Include custom headers
#include "libspm.h"
#include "globals.h"
#include "cutils.h"
#include "data.h"

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

    dbg(3, "Loading package %s from ALL_DB\n", i_pkg->name);

    // Allocate memory for package format and section
    char* pkg_format = calloc(64, sizeof(char));
    char* pkg_section = calloc(64, sizeof(char));

    // Retrieve data from the ALL_DB repository
    retrieve_data_repo(ALL_DB, i_pkg, &pkg_format, &pkg_section);

    dbg(3, "Got %s %s %s %s from DB", pkg_format, pkg_section, i_pkg->version, i_pkg->type);

    // Check if data retrieval was successful
    if (!pkg_format || !pkg_section || !i_pkg->version || !i_pkg->type) {
        msg(FATAL, "Failed to retrieve data from ALL_DB");
        return NULL;
    }

    dbg(3, "format is '%s'\n", pkg_format);
    dbg(3, "section is '%s'\n", pkg_section);

    dbg(1, "Downloading %s %s %s", i_pkg->name, i_pkg->version, i_pkg->type);

    // Construct the URL for the package download
    char url[64 + strlen(i_pkg->type) + strlen(i_pkg->name) + strlen(pkg_format)];
    sprintf(url, "%s/%s/%s.%s", pkg_section, i_pkg->type, i_pkg->name, pkg_format);

    // Download the package to the specified output path
    if (downloadRepo(url, out_path) != 0)
    {
        msg(ERROR, "Failed to download %s", url);
        return NULL;
    }

    // Free allocated memory
    free(pkg_section);
    return pkg_format;
}

// Function to synchronize the local repository with a remote repository
void sync()
{
    const char* filename = "/var/cccp/data/sources.list";
    int num_repos;
    Repos* repositories = read_sources_list(filename, &num_repos);
    if (repositories != NULL) {
        char* clone_directory = getenv("SOVIET_REPOS");
        if (clone_directory == NULL) {
            fprintf(stderr, "SOVIET_REPOS environment variable not set\n");
            free(repositories);
            return EXIT_FAILURE;
        }
        clone_repositories(repositories, num_repos, clone_directory);
        free(repositories);
    }
    return 0;
}
