#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <stdlib.h>

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
int open_pkg(const char* path, struct package* pkg, const char* format) {
    dbg(2, "Setting everything to NULL");
    // Set all variables to NULL
    memset(pkg, 0, sizeof(struct package));

    // Print make dependencies count
    dbg(3, "make dependencies count: %d", pkg->makedependenciesCount);

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
            printf("format: %s = %s\n", format, FORMATS[i]);
            if (strcmp(format, FORMATS[i]) == 0) {
                dbg(2, "Opening package with %s format", FORMATS[i]);
                strcat(path, ".ecmp");
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
int create_pkg(const char* path, struct package* pkg, const char* format) {
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
                dbg(2,"Opening package with %s format",FORMATS[i]);
                runFormatLib(FORMATS[i],"create",path,pkg);
                //free(*FORMATS);
                free(FORMATS);
                return 0;
            }
        }
    }
    msg(ERROR,"File %s is not a valid package file, or the format plugin isn't loaded",path);
    //free(*FORMATS);
    free(FORMATS);
    return -1;
}

// Load a format plugin, execute a specific function, and close the plugin
/*
Accepts:
- const char* format: The format of the package.
- const char* fn: The name of the function to execute in the format plugin.
- const char* pkg_path: The path to the package file.
- struct package* pkg: A pointer to the package structure.

Description:
This function loads a format plugin, executes a specified function within the plugin, and then closes the plugin.

Returns:
- int: An integer indicating the result of running the format plugin.
  - 0: Format plugin executed successfully.
  - 1: Format plugin file does not exist.
  - 1: Error loading or executing the format plugin.
  - -1: Format plugin function returned an error.
*/
int runFormatLib(const char* format, const char* fn, const char* pkg_path, struct package* pkg) {
    char lib_path[MAX_PATH];
    sprintf(lib_path, "%s/%s.so", getenv("SOVIET_PLUGIN_DIR"), format);
    dbg(2, "Loading %s", lib_path);

    if (access(lib_path, F_OK) != 0) {
        msg(ERROR, "File %s does not exist", lib_path);
        return 1;
    }

    // Load a function from the shared library
    void* handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }
    int (*func)(const char*, struct package*) = dlsym(handle, fn);
    char* error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }
    if (func(pkg_path, pkg) != 0) {
        return -1;
    }

    dlclose(handle);
    return 0;
}
