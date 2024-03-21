#include "stdio.h"
#include "unistd.h"
#include <stdlib.h>
#include <string.h>

// Include the necessary headers for the class file and utility functions
#include  "libspm.h"
#include "cutils.h"

// Function to uninstall packages
/*
Accepts:
- char* name: The name of the package to uninstall.

Returns:
- int: An integer indicating the result of the uninstallation.
  - 0: The package was successfully uninstalled.
  - -1: An error occurred during the uninstallation.

Description:
This function is used to uninstall packages. It relies on location data, which contains all the files that were installed by the program. This data is stored in a JSON array inside the package's SPM file in DATA_DIR. The function cycles through all the files in the JSON array and removes them from the system. It also removes the package's entry from the installed packages database.

Please avoid making changes to this code unless there's a critical bug or an important missing feature.
*/
int uninstall(char* name)
{
    // Get the SPM directory from the environment variables
    char* SPM_DIR = getenv("SOVIET_SPM_DIR");

    char** REPOS = calloc(512,sizeof(char));
    int REPO_COUNT = get_repos(REPOS);
    char* dataSpmPath = calloc(MAX_PATH, sizeof(char));

    for (int j = 0; j < REPO_COUNT; j++)
    {
        // Generate the path to the package's SPM file
        char* tmpSpmPath[MAX_PATH];
        sprintf(tmpSpmPath, "%s/%s/%s.%s", getenv("SOVIET_SPM_DIR"),REPOS[j], name, getenv("SOVIET_DEFAULT_FORMAT"));

        // Verify if the package is installed
        dbg(3, "Verifying if the package is installed at %s", tmpSpmPath);

        // Check if the SPM file exists
        if (access(tmpSpmPath, F_OK) == 0) {
            sprintf(dataSpmPath, "%s/%s/%s.%s", getenv("SOVIET_SPM_DIR"),REPOS[j], name, getenv("SOVIET_DEFAULT_FORMAT"));
            // Create a struct to store package information
            struct package r_pkg;

            // Open the package's SPM file and populate the r_pkg struct
            open_pkg(dataSpmPath, &r_pkg, NULL);

            dbg(3, "Found %d locations", r_pkg.locationsCount);

            // Remove all the files in the data["locations"]
            for (int i = 0; i < r_pkg.locationsCount; i++) {
                // Debug
                dbg(3, "Removing %s", r_pkg.locations[i]);
                dbg(3, "Remove exited with code %d", remove(r_pkg.locations[i]));
            }
            // Remove the SPM file from DATA_DIR
            remove(dataSpmPath);
        }
        else
        {
            msg(ERROR, "package not installed");
        }
    }
    return 0;
}
