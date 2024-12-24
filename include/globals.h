#include "stdbool.h"

#define QUEUE_MAX 256

#define MAX_PATH 2048
/*
START OF THE (sort of) CONSTANTS DECALRATIONS 
(They are not mean to be modified a lot)
*/

// enable testing mode
extern bool TESTING;
// enable verbose mode
extern bool QUIET;
// Flag indicating that no inputs are required
extern bool AUTO;
// Package queue
extern struct packages* PACKAGE_QUEUE;

