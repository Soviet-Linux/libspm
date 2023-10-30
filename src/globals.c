// Include necessary headers
#include "globals.h"
#include "cutils.h"
#include "stdbool.h"
#include <stdlib.h>

// Define a constant for the maximum queue size
#define QUEUE_MAX 64

// Declare global variables

// Flag for testing mode
bool TESTING = false;

// Flag for overwriting files
bool OVERWRITE = false;

// Flag for quiet mode (no verbose output)
bool QUIET = true;

// Database handles for installed and all packages
sqlite3 *INSTALLED_DB = NULL;
sqlite3 *ALL_DB = NULL;

// Array for package queue and its count
char *PACKAGE_QUEUE[QUEUE_MAX];
int QUEUE_COUNT;
