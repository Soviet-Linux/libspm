#include "cutils.h"         // Custom utility library
#include <stdlib.h>         // Standard library for general functions
#include <sys/stat.h>       // Standard library for file status information

// Function to clean working directories
/*
Returns:
- int: An integer indicating the result of the cleaning operation.
  - The return value is the sum of the following operations:
    - rmrf(build_dir): Removing the build directory and its contents.
    - rmrf(make_dir): Removing the make directory and its contents.
    - mkdir(build_dir, 0755): Creating the build directory with the specified permissions.
    - mkdir(make_dir, 0755): Creating the make directory with the specified permissions.
*/
int clean()
{
    // Get the paths to the build and make directories from environment variables
    char* build_dir = getenv("SOVIET_BUILD_DIR");
    char* make_dir = getenv("SOVIET_MAKE_DIR");

    // Clean the build and make directories, create them if they don't exist
    // Return the sum of return codes from rmrf (remove directory), mkdir (create directory)
    return rmrf(build_dir) + rmrf(make_dir) + mkdir(build_dir, 0755) + mkdir(make_dir, 0755);
}
