// This file is basically a copy of globals.h

#include "stdbool.h"
#include <stdlib.h>
#include "cutils.h"
#include "globals.h"

#define QUEUE_MAX 64

/*
START OF THE (sort of) CONSTANTS DECALRATIONS 
(They are not mean to be modified a lot)
*/



bool TESTING = false;

bool OVERWRITE = false;
bool QUIET  = true;

sqlite3* INSTALLED_DB = NULL;
sqlite3* ALL_DB = NULL;

char* PACKAGE_QUEUE[QUEUE_MAX];
int QUEUE_COUNT;


