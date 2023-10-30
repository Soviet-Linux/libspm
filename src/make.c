#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/stat.h>

// Include necessary headers
#include "cutils.h"
#include "globals.h"
#include "libspm.h"

// Function to build and install a package
/*
Accepts:
- char* package_dir: Path to the package directory.
- struct package* pkg: Pointer to the package structure.

Returns:
- int: An integer indicating the result of the build and installation process.
  - 0: Build and installation succeeded.
  - 1: An error occurred during execution of prepare, make, test, or install
commands.
  - -2: Failed to install the package.
  - -3: No install command found.
*/
int make(char *package_dir, struct package *pkg) {
  char *build_dir = getenv("SOVIET_BUILD_DIR");
  char *make_dir = getenv("SOVIET_MAKE_DIR");

  char *cmd_params;
  if (QUIET) {
    cmd_params = "&> /dev/null";
  } else {
    cmd_params = "";
  }

  // Set environment variables for building
  setenv("BUILD_ROOT", build_dir, 1);
  setenv("NAME", pkg->name, 1);
  setenv("VERSION", pkg->version, 1);

  // Extract URL from the package
  char excmd[64 + strlen(pkg->url)];
  sprintf(excmd, "echo -n %s", pkg->url);
  char *exurl = exec(excmd);
  setenv("URL", exurl, 1);
  free(exurl);

  // Download package sources
  if (pkg->info.download != NULL && strlen(pkg->info.download) > 0) {
    char sources_cmd[64 + strlen(make_dir) + strlen(pkg->info.download)];

    sprintf(sources_cmd, "(cd %s && %s) %s ", make_dir, pkg->info.download,
            cmd_params);
    dbg(2, "Downloading sources with %s", sources_cmd);
    int res = system(sources_cmd);

    if (res != 0) {
      msg(ERROR, "Failed to download sources for %s", pkg->name);
      return -1;
    }
  }

  // Run 'prepare' command
  if (pkg->info.prepare != NULL && strlen(pkg->info.prepare) > 0) {
    char prepare_cmd[64 + strlen(package_dir) + strlen(pkg->info.prepare) +
                     strlen(cmd_params)];

    sprintf(prepare_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.prepare,
            cmd_params);

    dbg(2, "Executing prepare command: %s", prepare_cmd);
    if (system(prepare_cmd) != 0) {
      return 1;
    }
    dbg(1, "Prepare command executed!");
  }

  // Run 'make' command
  if (pkg->info.make && strlen(pkg->info.make)) {
    char make_cmd[64 + strlen(package_dir) + strlen(pkg->info.make) +
                  strlen(cmd_params)];
    sprintf(make_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.make,
            cmd_params);

    dbg(2, "Executing make command: %s", make_cmd);
    if (system(make_cmd) != 0) {
      return 1;
    }
    dbg(1, "Make command executed!");
  }

  // Run 'test' command (if in testing mode)
  if (pkg->info.test != NULL && TESTING && strlen(pkg->info.test) > 0) {
    char test_cmd[64 + strlen(package_dir) + strlen(pkg->info.test) +
                  strlen(cmd_params)];
    sprintf(test_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.test,
            cmd_params);

    dbg(2, "Executing test command: %s", test_cmd);
    if (system(test_cmd) != 0) {
      return 1;
    }
    dbg(1, "Test command executed!");
  }

  // Run 'install' command
  if (pkg->info.install == NULL && strlen(pkg->info.install) == 0) {
    msg(ERROR, "No install command!");
    return -3;
  }

  char install_cmd[64 + strlen(package_dir) + strlen(pkg->info.install) +
                   strlen(cmd_params)];
  sprintf(install_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.install,
          cmd_params);

  dbg(2, "Executing install command: %s", install_cmd);
  if (system(install_cmd) != 0) {
    msg(FATAL, "Failed to install %s", pkg->name);
    return -2;
  }
  dbg(1, "Install command executed!");

  return 0;
}

// Function to execute a special command for post-installation
/*
Accepts:
- const char* cmd: The special command to execute.
- const char* package_dir: Path to the package directory.

Returns:
- int: An integer indicating the result of the special command execution.
  - 0: Special command executed successfully.
  - 1: An error occurred during special command execution.
*/
int exec_special(const char *cmd, const char *package_dir) {
  dbg(2, "Executing special command: %s");
  char *special_cmd =
      calloc(64 + strlen(package_dir) + strlen(cmd), sizeof(char));

  if (system(special_cmd) != 0) {
    free(special_cmd);
    return 1;
  }
  free(special_cmd);
  dbg(1, "Special command executed!");
  return 0;
}
