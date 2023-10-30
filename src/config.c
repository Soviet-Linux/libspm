#include "stdio.h"  // Standard I/O library for file operations
#include <stdlib.h> // Standard library for general functions
#include <string.h> // Standard library for string manipulation

#include "cutils.h"  // Custom utility library
#include "globals.h" // Custom library for global variables
#include "libspm.h"  // Custom library for package management

/*
This readConfig function takes a config file path as an argument and initializes
global variables with its content. The current implementation is considered
inefficient.
TODO: Rework it and use a HashMap to store the config values.

Accepts:
- const char* configFilePath: The path to the configuration file to be read.

Returns:
- int: An integer indicating the result of the configuration file reading
operation.
  - 0: Success.
  - 1: Invalid config file format.

Explanation of the config file format:
The config file contains key-value pairs, where each line represents a key-value
pair separated by an equal sign. The key-value pairs are used to set environment
variables. Here's an example of the config file format:

ROOT=/
MAIN_DIR=/var/cccp
DATA_DIR=/var/cccp/data
SPM_DIR=/var/cccp/spm
...etc...

The values are used to set corresponding environment variables, like
SOVIET_ROOT, SOVIET_MAIN_DIR, SOVIET_DATA_DIR, and so on. Unknown keys in the
config file are reported as errors.
*/
int readConfig(const char *configFilePath) {
  dbg(2, "config: %s", configFilePath);

  FILE *file = fopen(configFilePath, "r"); /* should check the result */
  char line[1024];
  char *kvlist[2];
  int count;

  while (fgets(line, sizeof(line), file)) {
    /* Note that fgets doesn't strip the terminating \n, checking its
       presence would allow handling lines longer than sizeof(line). */
    // Removing the '\n' mentioned above
    line[strlen(line) - 1] = 0;

    char *key = strtok(line, "=");
    char *value = strtok(NULL, "=");
    if (key == NULL || value == NULL) {
      msg(ERROR, "Invalid config file");
      return 1;
    }

    dbg(3, "Key: %s Value: %s", key, value);

    // Set environment variables based on the key-value pairs in the config file
    if (strcmp(key, "ROOT") == 0) {
      setenv("SOVIET_ROOT", value, 1);
    } else if (strcmp(key, "MAIN_DIR") == 0) {
      setenv("SOVIET_MAIN_DIR", value, 1);
    } else if (strcmp(key, "WORK_DIR") == 0) {
      setenv("SOVIET_WORK_DIR", value, 1);
    } else if (strcmp(key, "INSTALLED_DB") == 0) {
      setenv("SOVIET_INSTALLED_DB_PATH", value, 1);
    } else if (strcmp(key, "ALL_DB") == 0) {
      setenv("SOVIET_ALL_DB", value, 1);
    } else if (strcmp(key, "CONFIG_FILE") == 0) {
      setenv("SOVIET_CONFIG_FILE", value, 1);
    } else if (strcmp(key, "REPOS") == 0) {
      dbg(3, "REPOS: %s", value);
      setenv("SOVIET_REPOS", value, 1);
    } else if (strcmp(key, "FORMATS") == 0) {
      setenv("SOVIET_FORMATS", value, 1);
    } else {
      msg(ERROR, "Unknown key in config file: %s", key);
    }
  }

  fclose(file);

  return 0;
}
