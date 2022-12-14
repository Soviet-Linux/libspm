#include "stdio.h"
#include "unistd.h"
#include <sys/stat.h>

#include "libspm.h"
#include "utils.h"
#include "globals.h"
#include "data.h"


void init()
{



    clean();
    readConfig(CONFIG_FILE);
    // verify is all the DIR exists
    if (access(ROOT,F_OK)) mkdir(ROOT,0777);
    if (access(MAIN_DIR,F_OK)) mkdir(MAIN_DIR,0777);
    if (access(DATA_DIR,F_OK)) mkdir(DATA_DIR,0777);
    if (access(WORK_DIR,F_OK)) mkdir(WORK_DIR,0777);
    if (access(SPM_DIR,F_OK)) mkdir(SPM_DIR,0777);
    if (access(LOG_DIR,F_OK)) mkdir(LOG_DIR,0777);
    if (access(BUILD_DIR,F_OK)) mkdir(BUILD_DIR,0777);
    if (access(MAKE_DIR,F_OK)) mkdir(MAKE_DIR,0777);

    // init data
    // TODO: do some stuff for the data
    // init the databases
    connect_db(&INSTALLED_DB,INSTALLED_DB_PATH);
    create_table_installed(INSTALLED_DB);


    dbg(3,"ALL_DB_PATH: %s",ALL_DB_PATH);
    if (access(ALL_DB_PATH, F_OK) != 0) {
        msg(WARNING, "Global package data file not found, downloading...");
        sync();
    } else {
        connect_db(&ALL_DB,ALL_DB_PATH);
    }

    
    // Do other stuff if you want
}