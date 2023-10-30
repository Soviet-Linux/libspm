#include "limits.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Include necessary headers
#include "cutils.h"
#include "globals.h"

// Function to move binaries to the correct locations
/*
Accepts:
- char** locations: An array of file locations.
- long loc_size: The number of locations in the array.

Description:
This function iterates through the given file locations and moves the binaries
to their correct destinations.

Notes:
- It checks if the destination location is empty and moves files from the build
directory to the destination.
- If the destination location is not empty, it provides a warning and optionally
renames the file in the build directory.

Returns: None
*/
void move_binaries(char **locations, long loc_size) {
  // Iterate through locations and move the binaries to their correct locations
  for (int i = 0; i < loc_size; i++) {
    char dest_loc[PATH_MAX];
    sprintf(dest_loc, "%s/%s", getenv("SOVIET_ROOT"), locations[i]);
    char build_loc[PATH_MAX];
    sprintf(build_loc, "%s/%s", getenv("SOVIET_BUILD_DIR"), locations[i]);

    // Check if the destination location is empty
    if (!(access(dest_loc, F_OK) == 0)) {
      if (locations[i] == NULL) {
        msg(FATAL, "Location is NULL");
      }

      // Move the files from the build directory to the destination location
      mvsp(build_loc, dest_loc);

      msg(WARNING, "Moved %s/%s to %s", getenv("SOVIET_BUILD_DIR"),
          locations[i], dest_loc);
    } else {
      msg(WARNING, "%s is already here");

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
