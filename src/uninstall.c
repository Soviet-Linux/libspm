#include "stdio.h"
#include "unistd.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

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
int uninstall(struct package* pkg)
{
    char dataSpmPath[MAX_PATH];
    sprintf(dataSpmPath, "%s/%s", getenv("SOVIET_SPM_DIR"), pkg->path);

    // Check if the SPM file exists
    if (check(pkg) == 0) 
    {
        // Create a struct to store package information
        struct package r_pkg = {0};
        r_pkg.path = strdup(pkg->path);

        // Open the package's SPM file and populate the r_pkg struct
        open_pkg(getenv("SOVIET_SPM_DIR"), &r_pkg);

        dbg(3, "Found %d locations", r_pkg.locationsCount);
        // Remove all the files in the data["locations"]
        char loc_path[MAX_PATH];
        for (int i = 0; i < r_pkg.locationsCount; i++) 
        {
            sprintf(loc_path, "%s%s", getenv("SOVIET_ROOT"), r_pkg.locations[i]);

            dbg(3, "Removing %s", loc_path);                
            if (rmany(loc_path) != 0) {
                msg(ERROR,"Failed to remove %s",loc_path);
                perror("remove");
            }
        }
        // Remove the SPM file from DATA_DIR
        remove(dataSpmPath);
        free_pkg(&r_pkg);
        return 0;
    }

    msg(ERROR, "package not installed");
    return -1;
}


