#include "assert.h"
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

#include "../include/globals.h"
#include "../include/cutils.h"

#define STATIC

void test_check();
void test_clean();
void test_config();
void test_globals();
void test_init();
void test_install();
void test_make();
void test_move();
void test_pkg();
void test_repo();
void test_uninstall();
void test_update();
void test_util();

extern char WORKING_DIR[2048];

