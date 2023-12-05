#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "limits.h"

// Include necessary headers
#include "globals.h"
#include "cutils.h"

// Function to move binaries to the correct locations
/*
Accepts:
- char** locations: An array of file locations.
- long loc_size: The number of locations in the array.

Description:
This function iterates through the given file locations and moves the binaries to their correct destinations.

Notes:
- It checks if the destination location is empty and moves files from the build directory to the destination.
- If the destination location is not empty, it provides a warning and optionally renames the file in the build directory.

Returns: None
*/
void move_binaries(char** locations, long loc_size) {
    // Iterate through locations and move the binaries to their correct locations
    for (int i = 0; i < loc_size; i++) {
        char dest_loc[PATH_MAX];
        sprintf(dest_loc, "%s/%s", getenv("ROOT"), locations[i]);
        char build_loc[PATH_MAX];
        sprintf(build_loc, "%s/%s", getenv("SOVIET_BUILD_DIR"), locations[i]);

        // Check if the destination location is empty
        if (!(access(dest_loc, F_OK) == 0)) {
            if (locations[i] == NULL) {
                msg(FATAL, "Location is NULL");
            }

            // Move the files from the build directory to the destination location
            switch (mvsp(build_loc, dest_loc))
            {
                case -1:
                    msg(FATAL, "Moving %s/%s to %s failed, could not create dir", getenv("SOVIET_BUILD_DIR"), locations[i], dest_loc);
                    break;
                case -2:
                    msg(FATAL, "Moving %s/%s to %s failed, destination not a dir", getenv("SOVIET_BUILD_DIR"), locations[i], dest_loc);
                    break;
                case 0:
                    msg(WARNING, "Moved %s/%s to %s", getenv("SOVIET_BUILD_DIR"), locations[i], dest_loc);
                    break;
            }

            
        } else {
            msg(WARNING, "%s is already here, use --overwrite?", locations[i]);

            if (OVERWRITE) {
                // Rename the file in the build directory to the destination location
                rename(build_loc, dest_loc);
            } else {
                msg(FATAL, "Terminating the program");
            }
        }
    }
    return;
}

int better_mvsp(char* old_path,char* new_path)
{
    char* parent_path = calloc(strlen(new_path)+1,sizeof(char));
    strncpy(parent_path,new_path,strrchr(new_path, '/')-new_path);

    switch (isdir(parent_path))
    {
        case 1:
            if (mkdir_parent(parent_path, 0777) != 0) return -1;
            break;
        case 2:
            return -2;
        case 0:
            break;
    }
    free(parent_path);
    // move file
    return rename(old_path,new_path);
}

int mkdir_parent(const char *path, mode_t mode) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s", path);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
        if(*p == '/') {
            *p = 0;
            mkdir(tmp, mode);
            *p = '/';
        }
    mkdir(tmp, mode);

    return 0;
}
