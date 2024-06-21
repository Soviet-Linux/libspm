#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <linux/limits.h>

// Include necessary headers
#include "globals.h"
#include "libspm.h"
#include "cutils.h"

// Function to retrieve file locations within a directory
/*
Accepts:
- char*** locations: Pointer to an array of strings to store file locations.
- const char* loc_dir: Path to the directory to search for files.

Returns:
- long: The number of file locations retrieved.
*/
long get_locations(char*** locations, const char* loc_dir) {
  
  int num_files;
  *locations = getAllFiles(loc_dir+1, loc_dir, &num_files);
  // Log the count of retrieved locations for debugging
  dbg(2, "Got %d locations", num_files);

  // Return the count of file locations
  return num_files;
}
