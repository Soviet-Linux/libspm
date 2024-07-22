#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

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
        
        dbg(2, "Launching move for %s", locations[i]);

        char dest_loc[PATH_MAX];
        sprintf(dest_loc, "%s%s", getenv("SOVIET_ROOT"), locations[i]);
        char build_loc[PATH_MAX];
        sprintf(build_loc, "%s%s", getenv("SOVIET_BUILD_DIR"), locations[i]);

        // Check if the destination location is empty
        if (!(access(dest_loc, F_OK) == 0)) {
            if (locations[i] == NULL) {
                msg(FATAL, "Location is NULL");
            }

            // Move the files from the build directory to the destination location
            switch (mvsp(build_loc, dest_loc, getenv("SOVIET_BUILD_DIR")))
            {
                case -1:
                    msg(FATAL, "Moving %s to %s failed, could not create dir", build_loc, dest_loc);
                    break;
                case -2:
                    msg(FATAL, "Moving %s to %s failed, destination not a dir", build_loc, dest_loc);
                    break;
                case -3:
                    msg(WARNING, "Moving %s to %s failed, file absent, continuing...", build_loc, dest_loc);
                    break;
                case 0:
                    dbg(1, "Moved %s to %s", build_loc, dest_loc);
                    break;
            }
            
        } else {
            msg(WARNING, "%s is already here, use --overwrite?", locations[i]);
            if (OVERWRITE) {
                // Rename the file in the build directory to the destination location
                struct stat st;
                stat(build_loc, &st);
                int size = st.st_size;

                char* buffer = malloc(size);

                FILE *old_ptr;
                FILE *new_ptr;

                old_ptr = fopen(build_loc,"r"); 
                fread(buffer, sizeof(char), size, old_ptr); 
                fclose(old_ptr);

                new_ptr = fopen(dest_loc,"w"); 
                fwrite(buffer, sizeof(char), size, new_ptr); 
                fclose(new_ptr);
                
                chown(dest_loc, getuid(), getgid());
                chmod(dest_loc, 0755);

            } else {
                msg(FATAL, "Terminating the program");
            }
        }
    }
    return;
}