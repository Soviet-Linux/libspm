#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// class stuff
#include "spm/globals.h"
#include "spm/libspm.h"
#include "spm/utils.h"
#include <linux/limits.h>
#include <stdlib.h>

long get_locations(char ***locations, char *loc_dir) {
  // Get package file location
  char files_location_cmd[PATH_MAX + 64];
  sprintf(files_location_cmd,"( cd %s && find . -type f | cut -c2- ) ", BUILD_DIR);
  /*
  This way of getting locations is pretty bad , i should wite a proper way to do
  it But it works so its ok for now
  */
  msg(DBG2, "Getting files locations with %s ", files_location_cmd);

  // execute the commands
  // WARING : this is a memory leak
  char* res = exec(files_location_cmd);

  msg(DBG2, "Got locations : '%s'", res);
  
  // print the locations
  unsigned int max_count = ncountc(res,strlen(res),'\n');
  *locations = malloc(sizeof(char*) * max_count);
  printf("Found %d files\n", max_count);
  int count = splitm(res, '\n', *locations, max_count+1);


  printf("Got %d locations\n", count);

  /* WARNING : This is a bad way to do it , i should use a proper way to do it
   */
  //count -= 1;
  /* ARBITRARY NUMBER , i should find a better way to do it */
  // THIS ABOVE IS SOME REAL SHIT

  return count;
}