#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <linux/limits.h>

// Include necessary headers
#include "cutils.h"
#include "globals.h"
#include "libspm.h"

// Function to retrieve file locations within a directory
/*
Accepts:
- char*** locations: Pointer to an array of strings to store file locations.
- const char* loc_dir: Path to the directory to search for files.

Returns:
- long: The number of file locations retrieved.
*/
long get_locations(char ***locations, const char *loc_dir) {
  // Construct a shell command to list files in the specified directory
  char files_location_cmd[PATH_MAX + 64];
  sprintf(files_location_cmd, "( cd %s && find . -type f | cut -c2- ) ",
          loc_dir);

  // Log the constructed command for debugging
  dbg(2, "Getting files locations with %s ", files_location_cmd);

  // Execute the constructed shell command and store the output in 'res'
  char *res = exec(files_location_cmd);

  // Log the retrieved file locations for debugging
  dbg(3, "Got locations: '%s'", res);

  // Split the 'res' string into an array of file locations using '\n' as a
  // delimiter
  unsigned int count = splita(res, '\n', locations);

  // Log the count of retrieved locations for debugging
  dbg(2, "Got %d locations", count);

  // Return the count of file locations
  return count;
}
