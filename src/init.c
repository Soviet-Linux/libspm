#include "stdio.h"
#include "unistd.h"
#include <stdlib.h>
#include <sys/stat.h>

#include "libspm.h"
#include "cutils.h"
#include "globals.h"
#include "data.h"

// Function to initialize the Soviet Package Manager
void init() {
    char* debug_env = getenv("SOVIET_DEBUG");
    // Set the debugging level based on the environment variable
    dbg(3, "DEBUG: %s", debug_env);
    DEBUG += debug_env ? atoi(debug_env) : 0;
    dbg(3, "Initializing");

    // Set the configuration file path
    setenv("SOVIET_CONFIG_FILE", "/etc/cccp.conf", 0);
    setenv("SOVIET_REPOS_LIST", "/etc/sources.list", 0);
    dbg(3, "Cleaning...");
    readConfig(getenv("SOVIET_CONFIG_FILE"));
    //tht, do your thing here

    dbg(3, "Setting variables");

    // Set global variables for various paths and directories
    setenv("ROOT", "/", 0);
    setenv("MAIN_DIR", "/var/cccp", 0);
    setenv("SOVIET_REPOS_DIR", "/var/cccp/sources", 0);
    setenv("WORK_DIR", "/var/cccp/work", 0);
    setenv("SOVIET_SPM_DIR", "/var/cccp/spm", 0);
    setenv("SOVIET_LOG_DIR", "/var/cccp/log", 0);
    setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 0);
    setenv("SOVIET_BUILD_DIR", "/var/cccp/work/build", 0);
    setenv("SOVIET_MAKE_DIR", "/var/cccp/work/make", 0);
    setenv("SOVIET_TEST_LOG", "/var/cccp/log/test.log", 0);
   
    // Clean the working directories
    clean();

    // Check if all format plugins are installed
    char* formats_env = getenv("SOVIET_FORMATS");
    if (!formats_env) {
        msg(ERROR, "SOVIET_FORMATS environment variable not set");
        exit(1);
    }
    char** formats;
    int format_count = splita(strdup(formats_env), ' ', &formats);
    for (int i = 0; i < format_count; i++) {
        char* format = formats[i];
        char plugin[MAX_PATH];
        sprintf(plugin, "%s/%s.so", getenv("SOVIET_PLUGIN_DIR"), format);
        dbg(2, "Checking for %s plugin install", plugin);

        // Check if the format plugin exists
        if (access(plugin, F_OK)) {
            msg(ERROR, "Format plugin %s not found", format);
            exit(1);
        }
    }

    //free(*formats);
    free(formats);

    // Verify if required directories exist, and create them if not
    struct stat st = {0};
    if (stat(getenv("ROOT"), &st) == -1) {
        mkdir(getenv("ROOT"), 0777);
    }
    if (stat(getenv("MAIN_DIR"), &st) == -1) {
        mkdir(getenv("MAIN_DIR"), 0777);
    }
    if (stat(getenv("SOVIET_REPOS_DIR"), &st) == -1) {
        mkdir(getenv("SOVIET_REPOS_DIR"), 0777);
    }
    if (stat(getenv("WORK_DIR"), &st) == -1) {
        mkdir(getenv("WORK_DIR"), 0777);
    }
    if (stat(getenv("SOVIET_SPM_DIR"), &st) == -1) {
        mkdir(getenv("SOVIET_SPM_DIR"), 0777);
    }
    if (stat(getenv("SOVIET_LOG_DIR"), &st) == -1) {
        mkdir(getenv("SOVIET_LOG_DIR"), 0777);
    }
    if (stat(getenv("SOVIET_PLUGIN_DIR"), &st) == -1) {
        mkdir(getenv("SOVIET_PLUGIN_DIR"), 0777);
    }
    if (stat(getenv("SOVIET_BUILD_DIR"), &st) == -1) {
        mkdir(getenv("SOVIET_BUILD_DIR"), 0777);
    }
    if (stat(getenv("SOVIET_MAKE_DIR"), &st) == -1) {
        mkdir(getenv("SOVIET_MAKE_DIR"), 0777);
    }
}
