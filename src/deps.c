#include "stdio.h"          // Standard I/O library for file operations
#include <stdlib.h>         // Standard library for general functions
#include <string.h>         // Standard library for string manipulation
#include <unistd.h>         // Standard library for system calls
#include "limits.h"        // Provides PATH_MAX constant

// Custom header files
#include "libspm.h"         // Custom library for package management
#include "cutils.h"         // Custom utility library

// Function to check if all dependencies of a package are installed
/*
Accepts:
- char **dependencies: An array of dependency names.
- int dependenciesCount: The number of dependencies in the array.

Returns:
- int: An integer indicating the result of dependency checking.
  - 0: All dependencies are installed.
  - -1: An error occurred during dependency checking.
*/
int check_dependencies(char **dependencies, int dependenciesCount) {
    dbg(1, "Checking dependencies...");

    for (int i = 0; i < dependenciesCount; i++) {
        dbg(3, "Checking if %s is installed", dependencies[i]);

        if (!is_installed(dependencies[i])) {
            dbg(3, "Dependency %s is not installed", dependencies[i]);
            // TODO: We need to install the dependency
            msg(INFO, "Installing %s", dependencies[i]);

            // Check if the dependency is in the queue
            int in_queue = 0;
            for (int j = 0; j < QUEUE_COUNT; j++) {
                if (strcmp(PACKAGE_QUEUE[j], dependencies[i]) == 0) {
                    in_queue = 1;
                    break;
                }
            }

            if (in_queue) {
                dbg(1, "Package %s is already in the queue", dependencies[i]);
                continue;
            }

            char dep_path[PATH_MAX];
            snprintf(dep_path, PATH_MAX, "/tmp/%s-dep.tmp", dependencies[i]);

            struct package dep_pkg = {0};
            dep_pkg.name = dependencies[i];

            // Get the dependency
            char *dep_format = get(&dep_pkg, dep_path);

            if (dep_format == NULL) {
                msg(ERROR, "Failed to get dependency %s", dependencies[i]);
                return -1;
            }

            // Install the dependency
            /*
                TODO: Find a clever way to implement automatic dependency installing
                In the meantime, I'll implement no dependency checking.
            */

            return 0;
        } else {
            dbg(3, "Dependency %s is installed");
        }
    }

    return 0;
}
