#define _GNU_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Include necessary headers
#include "libspm.h"
#include "cutils.h"
#include "data.h"

// Function to install a package from source (archive)
/*
Accepts:
- const char* spm_path: Path to the package archive.
- int as_dep: Flag indicating if the package is a dependency.

Returns:
- int: An integer indicating the result of the installation.
  - 0: Package installed successfully.
  - -1: Installation failed.
  - -2: SOVIET_MAKE_DIR or SOVIET_BUILD_DIR is not set.
  - -3: Failed to open package.
  - -4: Failed to make the package.
  - -5: Failed to get locations for the package.
  - -6: Failed to store data in the installed database.

Meanings of return codes:
- 0: The package installation was successful, and the package is now installed on the system.
- -1: The package installation process encountered an error, and the installation failed.
- -2: The function requires the environment variables SOVIET_MAKE_DIR and SOVIET_BUILD_DIR to be set, but one or both of them are not set. This is an error condition.
- -3: The function attempted to open the package archive specified by spm_path, but the operation failed. This suggests an issue with the package archive.
- -4: The package build process (make) encountered an error and failed. The package could not be successfully built.
- -5: The function attempted to retrieve locations for the package, but it failed to do so. This indicates an issue with finding or determining the necessary package locations.
- -6: The function attempted to store data about the installed package in the database, but the operation failed. This suggests an issue with database storage.
*/
int install_package_source(const char* spm_path, int as_dep) {
    return f_install_package_source(spm_path, as_dep, NULL);
}

// Function to install a package from source with a specific format
/*
Accepts:
- const char* spm_path: Path to the package archive.
- int as_dep: Flag indicating if the package is a dependency.
- const char* format: Specific package format (optional).

Returns:
- int: An integer indicating the result of the installation.
  - 0: Package installed successfully.
  - -1: Installation failed.
  - -2: SOVIET_MAKE_DIR or SOVIET_BUILD_DIR is not set.
  - -3: Failed to open package.
  - -4: Failed to make the package.
  - -5: Failed to get locations for the package.
  - -6: Failed to store data in the installed database.
*/
int f_install_package_source(const char* spm_path, int as_dep, const char* format) {
    // Check if spm_path is NULL
    if (spm_path == NULL) {
        return -1;
    }

    // Get required directory paths
    char* make_dir = getenv("SOVIET_MAKE_DIR");
    char* build_dir = getenv("SOVIET_BUILD_DIR");

    if (make_dir == NULL || build_dir == NULL) {
        return -2;
    }

    // Initialize the package structure
    struct package pkg = {0};

    // Attempt to open the package archive
    if (open_pkg(spm_path, &pkg, format) != 0) {
        return -3;
    }

    // Add the package name to the queue
    PACKAGE_QUEUE[QUEUE_COUNT] = pkg.name;
    QUEUE_COUNT++;

    // Check package dependencies
    if (pkg.dependencies != NULL && pkg.dependenciesCount > 0 && strlen(pkg.dependencies[0]) > 0) {
        check_dependencies(pkg.dependencies, pkg.dependenciesCount);
    }

    // Checking makedeps
    if (pkg.makedependencies != NULL && pkg.makedependenciesCount > 0 && strlen(pkg.makedependencies[0]) > 0) {
        check_dependencies(pkg.makedependencies, pkg.makedependenciesCount);
    }

    // Legacy directory path for compatibility
    char legacy_dir[MAX_PATH];
    sprintf(legacy_dir, "%s/%s-%s", getenv("SOVIET_MAKE_DIR"), pkg.name, pkg.version);

    // Build the package
    if (make(legacy_dir, &pkg) != 0) {
        return -4;
    }

    // Get package locations
    pkg.locationsCount = get_locations(&pkg.locations, getenv("SOVIET_BUILD_DIR"));
    if (pkg.locationsCount <= 0) {
        return -5;
    }

    // Check if the package is already installed
    if (is_installed(pkg.name)) {
        uninstall(pkg.name);
    }

    // Move binaries to their destination
    move_binaries(pkg.locations, pkg.locationsCount);

    // Execute post-install scripts
    if (pkg.info.special != NULL && strlen(pkg.info.special) > 0) {
        exec_special(pkg.info.special, getenv("SOVIET_BUILD_DIR"));
    }

    // Format the path using sprintf
    char file_path[MAX_PATH];
    sprintf(file_path, "%s/%s.%s", getenv("SOVIET_SPM_DIR"), pkg.name, getenv("SOVIET_DEFAULT_FORMAT"));
    create_pkg(file_path, &pkg, NULL);

    // Store package data in the installed database
    if (store_data_installed(INSTALLED_DB, &pkg, as_dep) != 0) {
        return -6;
    }

    // Clean up
    clean();

    // Remove the package from the queue
    QUEUE_COUNT--;
    PACKAGE_QUEUE[QUEUE_COUNT] = NULL;

    // Free allocated memory
    free_pkg(&pkg);

    return 0;
}

// Utilities declaration for binary install
// Untar a binary package to another directory
/*
Accepts:
- const char* bin_path: Path to the binary package archive.
- const char* dest_dir: Destination directory for untarring.

Returns:
- int: An integer indicating the result of the untarring.
  - 0: Untarring completed successfully.
  - Non-zero: An error occurred during untarring.
*/
__attribute__((unused)) int uncompress_binary(const char* bin_path, const char* dest_dir);

// Function to install a package from a binary archive
/*
Accepts:
- const char* archivePath: Path to the binary archive.
- int as_dep: Flag indicating if the package is a dependency.

Returns:
- int: An integer indicating the result of the installation.
  - 0: Package installed successfully.
  - -1: Installation failed.
*/
int install_package_binary(const char* archivePath, int as_dep) {
    struct package pkg;

    // Get required environment variables
    char* default_format = getenv("SOVIET_DEFAULT_FORMAT");
    char* build_dir = getenv("SOVIET_BUILD_DIR");
    char* spm_dir = getenv("SOVIET_SPM_DIR");

    if (default_format == NULL || build_dir == NULL || spm_dir == NULL) {
        msg(ERROR, "Environment variables not set");
        return -1;
    }

    // Initialize package name
    pkg.name = calloc(sizeof(archivePath), sizeof(char));

    // Get the package name from the binary archive
    if (get_bin_name(archivePath, pkg.name) != 0) {
        msg(ERROR, "Could not get name from archive path");
        return -1;
    }

    // Uncompress binary and check the output
    if (uncompress_binary(archivePath, build_dir) != 0)
        return -1;

    // Format the path using sprintf
    char spm_path[MAX_PATH];
    sprintf(spm_path, "%s/%s.%s", build_dir, pkg.name, default_format);
    if (access(spm_path, F_OK) != 0) {
        msg(ERROR, "%s not found", spm_path);
        return -1;
    }

    // Open the package
    open_pkg(spm_path, &pkg, NULL);

    // Add the package name to the queue
    PACKAGE_QUEUE[QUEUE_COUNT] = pkg.name;
    QUEUE_COUNT++;
    dbg(1, "Added %s to QUEUE[%d]", pkg.name, QUEUE_COUNT - 1);

    // Check package dependencies
    check_dependencies(pkg.dependencies, pkg.dependenciesCount);

    // Move binaries to their destination
    dbg(1, "Moving binaries for %s", pkg.name);
    move_binaries(pkg.locations, pkg.locationsCount);

    // Execute post-install scripts
    exec_special(pkg.info.special, build_dir);

    // Format the path using sprintf
    char file_path[MAX_PATH];
    sprintf(file_path, "%s/%s.%s", spm_dir, pkg.name, default_format);
    create_pkg(file_path, &pkg, NULL);

    // Store package data in the installed database
    if (store_data_installed(INSTALLED_DB, &pkg, as_dep) != 0) {
        msg(ERROR, "Failed to store data in %s", INSTALLED_DB);
        msg(ERROR, "!! Package %s potentially corrupted !!", pkg.name);
        return -1;
    }
    dbg(1, "Package %s installed", pkg.name);

    // Clean up
    clean();

    // Remove the package from the queue
    QUEUE_COUNT--;
    PACKAGE_QUEUE[QUEUE_COUNT] = NULL;

    return 0;
}

// Function to check if a package is already installed
/*
Accepts:
- const char* name: Name of the package to check.

Returns:
- bool: A boolean value indicating whether the package is installed.
  - true: Package is installed.
  - false: Package is not installed.
*/
bool is_installed(const char* name) {
    char path[1024];
    char** FORMATS;
    int FORMAT_COUNT = splita(strdup(getenv("SOVIET_FORMATS")),' ',&FORMATS);

    // loop through all formats
    for (int i = 0; i < FORMAT_COUNT; i++)
    {
        sprintf(path,"%s/%s.%s",getenv("SOVIET_SPM_DIR"),name,FORMATS[i]);
        if (access(path,F_OK) == 0)
        {
            //free(*FORMATS);
            free(FORMATS);
            return true;
        }
    }
    return false;
}

// Function to get the package name from a binary archive path
/*
Accepts:
- const char* bin_path: Path to the binary package archive.
- char* name: A character array to store the package name.

Returns:
- int: An integer indicating the result of name extraction.
  - 0: Name extracted successfully.
  - -1: Extraction failed.
*/
int get_bin_name(const char* bin_path, char* name) {
    const char* file_name = strrchr(bin_path, '/');
    if (file_name == NULL)
        file_name = bin_path;
    else
        file_name++;
    for (int i = 0; i < (int)strlen(file_name); i++) {
        if (file_name[i] == '.') {
            sprintf(name, "%.*s", i, file_name);
            return 0;
        }
    }
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
int free_pkg(struct package* pkg) {
    if (pkg->name != NULL) free(pkg->name);
    if (pkg->version != NULL) free(pkg->version);
    if (pkg->license != NULL) free(pkg->license);
    if (pkg->type != NULL) free(pkg->type);
    if (pkg->url != NULL) free(pkg->url);

    if (pkg->info.make != NULL) free(pkg->info.make);
    if (pkg->info.special != NULL) free(pkg->info.special);
    if (pkg->info.download != NULL) free(pkg->info.download);
    if (pkg->info.install != NULL) free(pkg->info.install);
    if (pkg->info.prepare != NULL) free(pkg->info.install);
    if (pkg->info.test != NULL) free(pkg->info.test);

    if (pkg->locations) {
        if (*pkg->locations) free(*pkg->locations);
        free(pkg->locations);
    }

    if (pkg->dependencies) {
        if (*pkg->dependencies) free(*pkg->dependencies);
        free(pkg->dependencies);
    }
    if (pkg->makedependencies) {
        if (*pkg->makedependencies) free(*pkg->makedependencies);
        free(pkg->makedependencies);
    }
    if (pkg->makedependencies) {
        if (*pkg->makedependencies) free(*pkg->makedependencies);
        free(pkg->makedependencies);
    }
    return 0;
}

// Function to untar a binary package to a destination directory
/*
Accepts:
- const char* bin_path: Path to the binary package archive.
- const char* dest_dir: Destination directory for untarring.

Returns:
- int: An integer indicating the result of untarring.
  - 0: Untarring completed successfully.
  - Non-zero: An error occurred during untarring.
*/
int uncompress_binary(const char* bin_path, const char* dest_dir) {
    // Format the untar command using sprintf
    char untar_cmd[strlen(bin_path) + strlen(dest_dir) + 64];
    sprintf(untar_cmd, "tar -xvf %s -C %s", bin_path, dest_dir);

    // Execute the untar command
    return system(untar_cmd);
}
