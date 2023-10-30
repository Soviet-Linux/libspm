#include "stdio.h"
#include "unistd.h"
#include <stdlib.h>
#include <sys/stat.h>

#include "cutils.h"
#include "data.h"
#include "globals.h"
#include "libspm.h"

// Function to initialize the Soviet Package Manager
void init() {
  char *debug_env = getenv("SOVIET_DEBUG");

  // Set the debugging level based on the environment variable
  dbg(3, "DEBUG: %s", debug_env);
  DEBUG += debug_env ? atoi(debug_env) : 0;
  dbg(1, "Initializing");

  // Set the configuration file path
  setenv("SOVIET_CONFIG_FILE", "/etc/cccp.conf", 0);

  dbg(3, "Cleaning...");
  readConfig(getenv("SOVIET_CONFIG_FILE"));

  dbg(3, "Setting variables");

  // Set global variables for various paths and directories
  setenv("SOVIET_ROOT", "/", 0);
  setenv("SOVIET_MAIN_DIR", "/var/cccp", 0);
  setenv("SOVIET_DATA_DIR", "/var/cccp/data", 0);
  setenv("SOVIET_WORK_DIR", "/var/cccp/work", 0);
  setenv("SOVIET_SPM_DIR", "/var/cccp/spm", 0);
  setenv("SOVIET_LOG_DIR", "/var/cccp/log", 0);
  setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 0);
  setenv("SOVIET_BUILD_DIR", "/var/cccp/work/build", 0);
  setenv("SOVIET_MAKE_DIR", "/var/cccp/work/make", 0);
  setenv("SOVIET_INSTALLED_DB_PATH", "/var/cccp/data/installed.db", 0);
  setenv("SOVIET_ALL_DB_PATH", "/var/cccp/data/all.db", 0);
  setenv("SOVIET_TEST_LOG", "/var/cccp/log/test.log", 0);

  // Clean the working directories
  clean();

  // Check if all format plugins are installed
  char *formats_env = getenv("SOVIET_FORMATS");
  if (!formats_env) {
    msg(ERROR, "SOVIET_FORMATS environment variable not set");
    exit(1);
  }
  char **formats;
  int format_count = splita(strdup(formats_env), ' ', &formats);
  for (int i = 0; i < format_count; i++) {
    char *format = formats[i];
    char plugin[MAX_PATH];
    sprintf(plugin, "%s/%s.so", getenv("SOVIET_PLUGIN_DIR"), format);
    dbg(2, "Checking for %s plugin install", plugin);

    // Check if the format plugin exists
    if (access(plugin, F_OK)) {
      msg(ERROR, "Format plugin %s not found", format);
      exit(1);
    }
  }

  // free(*formats);
  free(formats);

  // Verify if required directories exist, and create them if not
  struct stat st = {0};
  if (stat(getenv("SOVIET_ROOT"), &st) == -1) {
    mkdir(getenv("SOVIET_ROOT"), 0777);
  }
  if (stat(getenv("SOVIET_MAIN_DIR"), &st) == -1) {
    mkdir(getenv("SOVIET_MAIN_DIR"), 0777);
  }
  if (stat(getenv("SOVIET_DATA_DIR"), &st) == -1) {
    mkdir(getenv("SOVIET_DATA_DIR"), 0777);
  }
  if (stat(getenv("SOVIET_WORK_DIR"), &st) == -1) {
    mkdir(getenv("SOVIET_WORK_DIR"), 0777);
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

  // Initialize the databases
  char *installed_db_path_env = getenv("SOVIET_INSTALLED_DB_PATH");
  if (!installed_db_path_env) {
    msg(ERROR, "SOVIET_INSTALLED_DB_PATH environment variable not set");
    exit(1);
  }
  connect_db(&INSTALLED_DB, installed_db_path_env);
  create_table_installed(INSTALLED_DB);

  char *all_db_path_env = getenv("SOVIET_ALL_DB_PATH");
  if (!all_db_path_env) {
    msg(ERROR, "SOVIET_ALL_DB_PATH environment variable not set");
    exit(1);
  }

  // Check if the global package data file exists, and download it if not
  if (access(all_db_path_env, F_OK) != 0) {
    msg(WARNING, "Global package data file not found, downloading...");
    sync();
  } else {
    connect_db(&ALL_DB, all_db_path_env);
  }
}
