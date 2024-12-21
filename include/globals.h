#include "stdbool.h"

#define QUEUE_MAX 256

#define MAX_PATH 2048
/*
START OF THE (sort of) CONSTANTS DECALRATIONS 
(They are not mean to be modified a lot)
*/


// enable testing mode
extern bool TESTING;
// overwrite file when installing
extern bool OVERWRITE;
// enable verbose mode
extern bool QUIET;
// Flag indicating that a user passed either Y or N to be used as default
extern bool OVERWRITE_CHOISE;
// Choise for passing N or Y to a prompt by default
extern char* USER_CHOISE[2];

extern char* PACKAGE_QUEUE[QUEUE_MAX];
extern int QUEUE_COUNT;

