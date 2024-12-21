#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "cutils.h"
#include "libspm.h"

// Open a package from the given path and populate the package structure
/*
Accepts:
- const char* path: The path to the package file.
- struct package* pkg: A pointer to the package structure to populate.
- const char* format: The format of the package (optional).

Description:
This function opens a package from the specified path, reads the package file's format, and populates the provided package structure with its contents.

Returns:
- int: An integer indicating the result of opening the package.
  - 0: Package opened successfully.
  - 1: File does not exist or is not a valid package file.
  - 1: File is not a valid package file or the format plugin isn't loaded.
*/
int open_pkg(const char* path, struct package* pkg, const char* format)
{
    dbg(2, "Setting everything to NULL");
    // Set all variables to NULL
    memset(pkg, 0, sizeof(struct package));

    // Print make dependencies count
    dbg(3, "make dependencies count: %d", pkg->optionalCount);
    dbg(3, "path: %s", path);

    // Check if the file exists
    if (access(path, F_OK) != 0) {
        msg(ERROR, "File %s does not exist\n", path);
        return 1;
    }

    // Check file extension
    if (format == NULL) {
        dbg(2, "Getting format from file extension");
        format = strrchr(path, '.') + 1;
        dbg(1, "Format: %s\n", format);
    }

    char** FORMATS;
    int FORMAT_COUNT = splita(getenv("SOVIET_FORMATS"), ' ', &FORMATS);

    if (format != NULL) {
        // This is experimental
        for (int i = 0; i < FORMAT_COUNT; i++) {
            dbg(2, "format: %s = %s\n", format, FORMATS[i]);
            if (strcmp(format, FORMATS[i]) == 0) {
                dbg(2, "Opening package with %s format", FORMATS[i]);
                runFormatLib(FORMATS[i], "open", path, pkg);
                return 0;
            }
        }
    } else {
        msg(ERROR, "File %s is not a valid package file", path);
        return 1;
    }
    msg(ERROR, "File %s is not a valid package file, or the format plugin isn't loaded", path);
    return 1;
}

// Create a package at the given path using the specified format and package structure
/*
Accepts:
- const char* path: The path to the package file to be created.
- struct package* pkg: A pointer to the package structure containing package data.
- const char* format: The format of the package (optional).

Description:
This function creates a package file at the specified path using the provided format and package data from the package structure.

Returns:
- int: An integer indicating the result of creating the package.
  - 0: Package created successfully.
  - -1: File is not a valid package file or the format plugin isn't loaded.
*/
int create_pkg(struct package* pkg, const char* format) 
{
    char repo_path[MAX_PATH];
    sprintf(repo_path, "%s/%s", getenv("SOVIET_SPM_DIR"), pkg->repo);

    if(isdir(repo_path) != 0)
    { 
        pmkdir(repo_path);
    }
    
    char path[MAX_PATH]; 
    sprintf(path, "%s/%s/%s.%s", getenv("SOVIET_SPM_DIR"), pkg->repo, pkg->name, getenv("SOVIET_DEFAULT_FORMAT"));

    msg(INFO, "Creating package %s", path);

    char** FORMATS;
    int FORMAT_COUNT = splita(strdup(getenv("SOVIET_FORMATS")),' ',&FORMATS);

    // get file extension
    if (format == NULL)
    {
        format = strrchr( path, '.' ) + 1;
    } 
    /* This illustrates strrchr */
    if (format != NULL)
    {
        // this is experimental
        for (int i = 0; i < FORMAT_COUNT; i++)
        {
            if (strcmp(format,FORMATS[i]) == 0)
            {
                dbg(2, "Opening package with %s format", FORMATS[i]);
                runFormatLib(FORMATS[i], "create", path, pkg);
                //free(*FORMATS);
                return 0;
            }
        }
    }
    msg(ERROR,"File %s is not a valid package file, or the format plugin isn't loaded",path);
    //free(*FORMATS);
    free(FORMATS);
    return -1;
}

// Function to free memory allocated for a package structure
/*
Accepts:
- struct package* pkg: Pointer to a package structure.

Returns:
- int: An integer indicating the result of memory deallocation.
  - 0: Memory freed successfully.
*/
int free_pkg(struct package* pkg) 
{
    if (pkg->name != NULL) free(pkg->name);
    if (pkg->version != NULL) free(pkg->version);
    if (pkg->license != NULL) free(pkg->license);
    if (pkg->type != NULL) free(pkg->type);
    if (pkg->url != NULL) free(pkg->url);

    if (pkg->info.make != NULL) free(pkg->info.make);
    if (pkg->info.special != NULL) free(pkg->info.special);
    if (pkg->info.download != NULL) free(pkg->info.download);
    if (pkg->info.install != NULL) free(pkg->info.install);
    if (pkg->info.prepare != NULL) free(pkg->info.prepare);
    if (pkg->info.test != NULL) free(pkg->info.test);

    if (pkg->locations) {
        if (*pkg->locations) free(*pkg->locations);
        free(pkg->locations);
    }
    if (pkg->dependencies) {
        if (*pkg->dependencies) free(*pkg->dependencies);
        free(pkg->dependencies);
    }
    if (pkg->optional) {
        if (*pkg->optional) free(*pkg->optional);
        free(pkg->optional);
    }
    if (pkg->files) {
        if (*pkg->files) free(*pkg->files);
        free(pkg->files);
    }
    return 0;
}

int create_package_db(){}