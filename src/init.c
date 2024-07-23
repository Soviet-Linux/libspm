#include "stdio.h"
#include "unistd.h"
#include <stdlib.h>
#include <sys/stat.h>

#include "libspm.h"
#include "cutils.h"
#include "globals.h"

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

    dbg(3, "Setting variables");

    // Set global variables for various paths and directories
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
    if (stat(getenv("SOVIET_ROOT"), &st) == -1) {
        pmkdir(getenv("SOVIET_ROOT"));
    }
    if (stat(getenv("SOVIET_MAIN_DIR"), &st) == -1) {
        pmkdir(getenv("SOVIET_MAIN_DIR"));
    }
    if (stat(getenv("SOVIET_REPOS_DIR"), &st) == -1) {
        pmkdir(getenv("SOVIET_REPOS_DIR"));
    }
    if (stat(getenv("SOVIET_WORK_DIR"), &st) == -1) {
        pmkdir(getenv("SOVIET_WORK_DIR"));
    }
    if (stat(getenv("SOVIET_SPM_DIR"), &st) == -1) {
        pmkdir(getenv("SOVIET_SPM_DIR"));
    }
    if (stat(getenv("SOVIET_LOG_DIR"), &st) == -1) {
        pmkdir(getenv("SOVIET_LOG_DIR"));
    }
    if (stat(getenv("SOVIET_PLUGIN_DIR"), &st) == -1) {
        pmkdir(getenv("SOVIET_PLUGIN_DIR"));
    }
    if (stat(getenv("SOVIET_BUILD_DIR"), &st) == -1) {
        pmkdir(getenv("SOVIET_BUILD_DIR"));
    }
    if (stat(getenv("SOVIET_MAKE_DIR"), &st) == -1) {
        pmkdir(getenv("SOVIET_MAKE_DIR"));
    }
}
