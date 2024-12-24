// Include necessary header files for the code
#include "libspm.h"       // Custom library for package management
#include "cutils.h"

#include <unistd.h>       // Standard library for system calls
#include <stdio.h>        // Standard I/O library for file operations
#include <stdlib.h>      // Standard library for general functions

// Function to check if a package is installed and untouched
/*
Accepts:
- const char* name: The name of the package to be checked.

Returns:
- int: An integer indicating the result of the check.
  - 0: Good, package is installed and fine.
  - 1: Package is not installed (Package data file is absent).
  - 2: Package is corrupted (package data file is here but with no location info).
  - 3: Package is corrupted (Some locations aren't here).
*/
int check(struct package* pkg)
{
    // Create the path to the package data file using environment variables
    dbg(2, "Creating a path to the local package");

    char dataSpmPath[MAX_PATH];
    sprintf(dataSpmPath, "%s/%s", getenv("SOVIET_SPM_DIR"), pkg->path);
    dbg(2, "PATH: %s", dataSpmPath);

    // Checking if the package data file exists
    if (access(dataSpmPath, F_OK) != 0) {
        dbg(2, "PATH does not exist");
        return 1; // Exit code 1 indicates the package is not installed
    }

    // Open the package data file and load its contents
    dbg(2, "Opening the package");
    open_pkg(getenv("SOVIET_SPM_DIR"), pkg);

    dbg(2, "Checking package locations");
    // If the package data file lacks location information, return exit code 2
    if (pkg->locationsCount == 0) {
        dbg(2, "The package is corrupted");
        return 2; // Exit code 2 indicates the package is corrupted (no location info)
    }

    // Check the existence of package locations and return the appropriate exit code
    return check_locations(pkg->locations, pkg->locationsCount);
}

// Function to check the existence of package locations
/*
Accepts:
- char** locations: An array of strings representing package locations.
- int locationsCount: The number of locations in the array.

Returns:
- int: An integer indicating the result of the check.
  - 0: All locations exist, so the package is installed and fine.
  - 3: Some locations do not exist, indicating package corruption (some locations are missing).
*/
int check_locations(char** locations, int locationsCount)
{
    for (int i = 0; i < locationsCount; i++) {
        // If a location doesn't exist, return exit code 3
        if (access(locations[i], F_OK) != 0) {
            dbg(2, "The package is corrupted (some locations are missing)");
            return 3; // Exit code 3 indicates the package is corrupted (some locations are missing)
        }
    }

    // All locations exist, so return exit code 0 (package is installed and fine)
    return 0;
}
