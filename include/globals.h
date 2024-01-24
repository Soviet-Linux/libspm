#include "stdbool.h"
#include <sqlite3.h>

#define QUEUE_MAX 64

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
// Flag for skipping checksum validation
extern bool INSECURE;


extern sqlite3* INSTALLED_DB;
extern sqlite3* ALL_DB;

extern char* PACKAGE_QUEUE[QUEUE_MAX];
extern int QUEUE_COUNT;

