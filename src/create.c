#include <stdio.h>         // Standard I/O library for file operations
#include <stdlib.h>        // Standard library for general functions
#include <string.h>        // Standard library for string manipulation

#include "libspm.h"        // Custom library for package management
#include "cutils.h"        // Custom utility library

// Function prototype for creating an archive from a directory
int create_archive(const char* DIR, const char* out_path);

// Function to create a binary package from source
/*
Accepts:
- const char* spm_path: The path to the source package.
- const char* bin_path: The path where the binary package will be created.

Returns:
- int: An integer indicating the result of the binary package creation.
*/
int create_binary_from_source(const char* spm_path, const char* bin_path) {
    return f_create_binary_from_source(spm_path, bin_path, NULL, getenv("SOVIET_DEFAULT_FORMAT"));
}

// Function to create a binary package from source with input and output formats
/*
Accepts:
- const char* src_path: The path to the source package.
- const char* bin_path: The path where the binary package will be created.
- const char* in_format: The input format (optional).
- const char* out_format: The output format.

Returns:
- int: An integer indicating the result of the binary package creation.
*/
int f_create_binary_from_source(const char* src_path, const char* bin_path, const char* in_format, const char* out_format)
{
    struct package pkg;

    (void)in_format;
    (void)out_format;
    
    // Open the package source
    open_pkg(src_path, &pkg, NULL);

    PACKAGE_QUEUE[QUEUE_COUNT] = pkg.name; // Add the package name to the PKG_QUEUE ARRAY
    QUEUE_COUNT++;
    dbg(1, "Added %s at QUEUE[%d] ", pkg.name, QUEUE_COUNT);

    /* 
        Here we have some problems:
        The legacy package directory was in MAKE_DIR/$NAME-$VERSION
        Should we keep it or not? 
        I choose, for compatibility reasons, to keep it.
        If someone wants to change this, you can vote here:
         - keep it: 1
         - change it: 0
    */
    const char* MAKE_DIR = getenv("SOVIET_MAKE_DIR");
    const char* BUILD_DIR = getenv("SOVIET_BUILD_DIR");

    char legacy_dir[MAX_PATH];
    sprintf(legacy_dir, "%s/%s-%s", MAKE_DIR, pkg.name, pkg.version);
    dbg(1, "legacy dir: %s", legacy_dir);

    // Make the package
    if (make(legacy_dir, &pkg) != 0) {
        msg(FATAL, "Make failed");
    }
    dbg(1, "Make done - %s", pkg.name);

    // Get package locations
    dbg(1, "Getting locations - %s", pkg.name);
    pkg.locationsCount = get_locations(&pkg.locations, BUILD_DIR);

    char file_path[MAX_PATH];
    sprintf(file_path, "%s/%s.%s", BUILD_DIR, pkg.name, getenv("SOVIET_DEFAULT_FORMAT"));

    // Create a package file
    create_pkg(file_path, &pkg, NULL);

    // Compress binaries to a package archive
    dbg(1, "Compressing binaries - %s", pkg.name);
    create_archive(BUILD_DIR, bin_path);

    // Clean working directories
    clean();

    return 0;
}

// Function to create an archive from a directory
/*
Accepts:
- const char* DIR: The directory to be archived.
- const char* out_path: The path where the archive file will be created.

Returns:
- int: An integer indicating the result of the archive creation.
*/
int create_archive(const char* DIR, const char* out_path)
{
    char* archive_cmd = calloc(256, sizeof(char));
    sprintf(archive_cmd, "( cd %s && tar -czf %s . )", DIR, out_path);
    dbg(1, "archive_cmd: %s", archive_cmd);
    int EXIT = system(archive_cmd);
    free(archive_cmd);
    return EXIT;
}
