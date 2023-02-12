#include "stdio.h"
#include "unistd.h"
#include <stdlib.h>
#include <sys/stat.h>

#include "libspm.h"
#include "cutils.h"
#include "globals.h"
#include "data.h"


void init()
{
    DEBUG += atoi(getenv("SOVIET_DEBUG"));
    printf("DEBUG: %d\n",DEBUG);
    dbg(1,"Initializing");

    setenv("SOVIET_CONFIG_FILE","/etc/cccp.conf",0);

    clean();
    readConfig(getenv("SOVIET_CONFIG_FILE"));

    // set all the global variables
    setenv("SOVIET_ROOT","/",0);
    setenv("SOVIET_MAIN_DIR","/var/cccp",0);
    setenv("SOVIET_DATA_DIR","/var/cccp/data",0);
    setenv("SOVIET_WORK_DIR","/var/cccp/work",0);
    setenv("SOVIET_SPM_DIR","/var/cccp/spm",0);
    setenv("SOVIET_LOG_DIR","/var/cccp/log",0);
    setenv("SOVIET_PLUGIN_DIR","/var/cccp/plugins",0);

    setenv("SOVIET_BUILD_DIR","/var/cccp/work/build",0);
    setenv("SOVIET_MAKE_DIR","/var/cccp/work/make",0);

    setenv("SOVIET_INSTALLED_DB_PATH","/var/cccp/data/installed.db",0);
    setenv("SOVIET_ALL_DB_PATH","/var/cccp/data/all.db",0);

    setenv("SOVIET_TEST_LOG","/var/cccp/log/test.log",0);

    // check if all format plugins are installed
    char** formats; 
    int format_count = splita(strdup(getenv("SOVIET_FORMATS")),' ',&formats);
    for (int i = 0; i < format_count; i++) {
        char* format = formats[i];
        char plugin[MAX_PATH];
        sprintf(plugin,"%s/%s.so",getenv("SOVIET_PLUGIN_DIR"),format);
        if (access(plugin,F_OK)) {
            msg(ERROR,"Format plugin %s not found",format);
            exit(1);
        }
    }
    free(*formats);
    free(formats);


    setenv("SOVIET_DEFAULT_FORMAT",formats[0],0);

    // verify is all the DIR exists
    if (access(getenv("SOVIET_ROOT"),F_OK)) mkdir(getenv("SOVIET_ROOT"),0777);
    if (access(getenv("SOVIET_MAIN_DIR"),F_OK)) mkdir(getenv("SOVIET_MAIN_DIR"),0777);
    if (access(getenv("SOVIET_DATA_DIR"),F_OK)) mkdir(getenv("SOVIET_DATA_DIR"),0777);
    if (access(getenv("SOVIET_WORK_DIR"),F_OK)) mkdir(getenv("SOVIET_WORK_DIR"),0777);
    if (access(getenv("SOVIET_SPM_DIR"),F_OK)) mkdir(getenv("SOVIET_SPM_DIR"),0777);
    if (access(getenv("SOVIET_LOG_DIR"),F_OK)) mkdir(getenv("SOVIET_LOG_DIR"),0777);
    if (access(getenv("SOVIET_PLUGIN_DIR"),F_OK)) mkdir(getenv("SOVIET_PLUGIN_DIR"),0777);
    if (access(getenv("SOVIET_BUILD_DIR"),F_OK)) mkdir(getenv("SOVIET_BUILD_DIR"),0777);
    if (access(getenv("SOVIET_MAKE_DIR"),F_OK)) mkdir(getenv("SOVIET_MAKE_DIR"),0777);


    // init data
    // TODO: do some stuff for the data
    // init the databases
    connect_db(&INSTALLED_DB,getenv("SOVIET_INSTALLED_DB_PATH"));
    create_table_installed(INSTALLED_DB);

    char* all_db_path = getenv("SOVIET_ALL_DB_PATH");

    dbg(3,"ALL_DB_PATH: %s",all_db_path);
    if (access(all_db_path, F_OK) != 0) {
        msg(WARNING, "Global package data file not found, downloading...");
        sync();
    } else {
        connect_db(&ALL_DB,all_db_path);
    }
    
   return;
}