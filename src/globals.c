// Include necessary headers
#include "stdbool.h"
#include <stdlib.h>
#include "cutils.h"
#include "globals.h"

// Define a constant for the maximum queue size
#define QUEUE_MAX 64

// Declare global variables

// Flag for testing mode
bool TESTING = false;

// Flag for overwriting files
bool OVERWRITE = false;

// Flag for quiet mode (no verbose output)
bool QUIET = true;

// Flag for skipping checksum validation
bool INSECURE = false;

// Flag indicating that a user passed either Y or N to be used as default
bool OVERWRITE_CHOISE = false;

// Choise for passing N or Y to a prompt by default
char* USER_CHOISE[2];

// Array for package queue and its count
char* PACKAGE_QUEUE[QUEUE_MAX];
int QUEUE_COUNT;
