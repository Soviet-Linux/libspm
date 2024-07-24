#include "assert.h"
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

#include "../include/globals.h"
#include "../include/cutils.h"


#define STATIC


void test_move();
void test_get();
void test_split();
void test_config();

void test_ecmp(char* spm_path);
void test_make(char* spm_path);
void test_pm(char* spm_path);

extern char WORKING_DIR[2048];

