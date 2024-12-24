// Include necessary headers
#include "stdbool.h"
#include "libspm.h"
#include <stdlib.h>
#include "cutils.h"
#include "globals.h"

// Define a constant for the maximum queue size
#define QUEUE_MAX 256

// Declare global variables

// Flag for testing mode
bool TESTING = false;

// Flag for overwriting files
bool OVERWRITE = false;

// Flag for quiet mode (no verbose output)
bool QUIET = true;

// Flag indicating that no inputs are required
bool AUTO = false;

// Array for package queue and its count
struct packages* PACKAGE_QUEUE = {0};
