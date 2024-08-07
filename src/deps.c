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

            struct package* pkg = calloc(1, sizeof(struct package));
            pkg->name = dependencies[i];

            char* pkg_name = calloc(strlen(pkg->name) + 1, sizeof(char));
            if(!strstr(pkg->name, ".ecmp"))
            {
                sprintf(pkg_name, "%s.%s", pkg->name, getenv("SOVIET_DEFAULT_FORMAT"));
            }
                else
                {
                    pkg_name = strdup(pkg->name);
                }

            int num_results;
            char** results = search(pkg_name, &num_results);
                
            char* format;

            if(results != NULL)
            {
                for ( int i = 0; i < num_results; i++)
                {
                    // Package name
                    char* temp_1 = strtok(results[i], ">");
                    // Repo it's in
                    char* temp_2 = strchr(results[i], '\0') + 1;

                    if(strcmp(getenv("SOVIET_DEFAULT_REPO"), temp_2) == 0)
                    {
                        format = temp_2;
                        break;
                    }
                        else if (i == num_results) 
                        {
                            format = temp_2;
                        }
                }
            }
            
            if (format == NULL) {
                msg(ERROR, "Failed to download package %s", pkg->name);
                return 1;
            }

            get(pkg, format, pkg->name);

            f_install_package_source(pkg->name, 0, format);

            remove(pkg_name);

        } else {
            dbg(3, "Dependency %s is installed", dependencies[i]);
        }
    }

    return 0;
}


// Function to check if all optional dependencies of a package are installed
/*
Accepts:
- char **dependencies: An array of dependency names.
- int dependenciesCount: The number of dependencies in the array.

Returns:
- int: An integer indicating the result of dependency checking.
  - 0: All dependencies are installed.
  - -1: An error occurred during dependency checking.
*/
int check_optional_dependencies(char **dependencies, int dependenciesCount) {
    dbg(1, "Checking optional dependencies...");

    for (int i = 0; i < dependenciesCount; i++) {
        dbg(3, "Checking if %s is installed", dependencies[i]);
        if (!is_installed(dependencies[i])) {
            dbg(3, "Dependency %s is not installed", dependencies[i]);


            char* str = calloc(2, sizeof(char));

            msg(INFO, "Do you want to download optional package %s, y/N", dependencies[i]);
            if(!OVERWRITE_CHOISE)
            {
                char* res = fgets(str, 2, stdin);

                if ( strchr(str, '\n') == NULL )
                {
                    while ((getchar()) != '\n');
                }

                int k = 0;

                while (str[k] != '\n' && str[k] != '\0')
                {
                    k++;
                }

                if (str[k] == '\n')
                {
                    str[k] = '\0';
                }
            }
                else
                {
                    sprintf(str, "%s", USER_CHOISE[0]);
                }

            if((strcmp(str, "Y") == 0 || strcmp(str, "y") == 0))
            {
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

                struct package* pkg = calloc(1, sizeof(struct package));
                pkg->name = dependencies[i];

                char* format;
                char* pkg_name = calloc(strlen(pkg->name) + 1, sizeof(char));
                if(!strstr(pkg->name, ".ecmp"))
                {
                    sprintf(pkg_name, "%s.%s", pkg->name, getenv("SOVIET_DEFAULT_FORMAT"));
                }
                    else
                    {
                        pkg_name = strdup(pkg->name);
                    }

                int num_results;
                char** results = search(pkg_name, &num_results);
                    
                if(results != NULL)
                {
                    for ( int i = 0; i < num_results; i++)
                    {
                        // Package name
                        char* temp_1 = strtok(results[i], ">");
                        // Repo it's in
                        char* temp_2 = strchr(results[i], '\0') + 1;

                        if(strcmp(getenv("SOVIET_DEFAULT_REPO"), temp_2) == 0)
                        {
                            format = temp_2;
                            break;
                        }
                            else if (i == num_results) 
                            {
                                format = temp_2;
                            }
                    }
                }
                
                if (format == NULL) {
                    msg(ERROR, "Failed to download package %s", pkg->name);
                    return 1;
                }

                if (format == NULL) {
                msg(ERROR, "Failed to download package %s", pkg->name);
                return 1;
                }

                get(pkg, format, pkg->name);

                f_install_package_source(pkg->name, 0, format);

                remove(pkg_name);
            }
            else
            {
                msg(INFO, "Skipping %s", dependencies[i]);
            }
            free(str);

        } else {
            dbg(3, "Dependency %s is installed", dependencies[i]);
        }
    }

    return 0;
}
