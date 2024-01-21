#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/stat.h>

// Include necessary headers
#include "libspm.h"
#include "cutils.h"
#include "globals.h"

// Function to build and install a package
/*
Accepts:
- char* package_dir: Path to the package directory.
- struct package* pkg: Pointer to the package structure.
- int skip_checksum: a int value to determine if the checksum should be bypassed
   - Valid Values for skip_checksum:
     -  0: To not skip the checksum
     -  1: To skip the checksum
     -  Any other value will result to the checksum not being bypassed

Returns:
- int: An integer indicating the result of the build and installation process.
  - 0: Build and installation succeeded.
  - 1: An error occurred during execution of prepare, make, test, or install commands.
  - -2: Failed to install the package.
  - -3: No install command found.
*/
int make(char* package_dir, struct package* pkg, int skip_checksum) {
    char* build_dir = getenv("SOVIET_BUILD_DIR");
    char* make_dir = getenv("SOVIET_MAKE_DIR");

    char* cmd_params;
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
    char* exurl = exec(excmd);
    setenv("URL", exurl, 1);
    free(exurl);

    // Download package sources
    if (pkg->info.download != NULL && strlen(pkg->info.download) > 0) {
        char sources_cmd[64 + strlen(make_dir) + strlen(pkg->info.download)];

        sprintf(sources_cmd, "(cd %s && %s) %s ", make_dir, pkg->info.download, cmd_params);
        dbg(2, "Downloading sources with %s", sources_cmd);
        int res = system(sources_cmd);

        if (res != 0) {
            msg(ERROR, "Failed to download sources for %s", pkg->name);
            return -1;
        }
    }

    if ((pkg->sha256 == NULL) || (pkg->sha256[0] == '\0'))
    {
        pkg->sha256 = "Not provided";
    }

    // Check if the checksum shall be bypassed
    if (skip_checksum != 1){
      // Check the hash, abort if mismatch
      char* exec_cmd_1 = calloc(MAX_PATH, sizeof(char));
      char* exec_cmd_2 = calloc(MAX_PATH, sizeof(char));

      sprintf(exec_cmd_1, "( cd %s && find . -maxdepth 1 -type f  | cut -c3- ) ", getenv("SOVIET_MAKE_DIR"));
      dbg(1, "Executing  %s to search for package tarball", exec_cmd_1);
      char* file = exec(exec_cmd_1);

      file[strcspn(file, "\n")] = 0;

      if (!((file == NULL) || (file[0] == '\0')))
        {
        dbg(1, "Checking file %s, if this is not the package tarball, the author of this line is stupid", file);

        sprintf(exec_cmd_2, "( cd %s && sha256sum %s | cut -d ' ' -f 1)", getenv("SOVIET_MAKE_DIR"), file);
        dbg(1, "Executing  %s to check the hash", exec_cmd_2);
        char* hash = exec(exec_cmd_2);

        if (!((hash == NULL) || (hash[0] == '\0')))
            {
          dbg(1, "Hash is %s", pkg->sha256);
          dbg(1, "Got %s", hash);

          hash[strcspn(hash, "\n")] = 0;


          if(strcmp(hash, pkg->sha256) != 0)
                {
            msg(FATAL, "Hash mismatch, aborting, use --no-checksum? (not present atm)");
          }
        }
      }
        else
        {
        msg(FATAL, "Could not verify the file's hash, might be the package fault, use --no-checksum? (not present atm)");
      }

      free(exec_cmd_1);
      free(exec_cmd_2);
    } else {
      msg(WARNING, "The Checksum is being skipped");
    }
    // Run 'prepare' command
    if (pkg->info.prepare != NULL && strlen(pkg->info.prepare) > 0) {
        char prepare_cmd[64 + strlen(package_dir) + strlen(pkg->info.prepare) + strlen(cmd_params)];

        sprintf(prepare_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.prepare, cmd_params);

        dbg(2, "Executing prepare command: %s", prepare_cmd);
        if (system(prepare_cmd) != 0) {
            return 1;
        }
        dbg(1, "Prepare command executed!");
    }

    // Run 'make' command
    if (pkg->info.make && strlen(pkg->info.make)) {
        char make_cmd[64 + strlen(package_dir) + strlen(pkg->info.make) + strlen(cmd_params)];
        sprintf(make_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.make, cmd_params);

        dbg(2, "Executing make command: %s", make_cmd);
        if (system(make_cmd) != 0) {
            return 1;
        }
        dbg(1, "Make command executed!");
    }

    // Run 'test' command (if in testing mode)
    if (pkg->info.test != NULL && TESTING && strlen(pkg->info.test) > 0) {
        char test_cmd[64 +  strlen(package_dir) + strlen(pkg->info.test) + strlen(cmd_params)];
        sprintf(test_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.test, cmd_params);

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

    char install_cmd[64 + strlen(package_dir) + strlen(pkg->info.install) + strlen(cmd_params)];
    sprintf(install_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.install, cmd_params);

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
int exec_special(const char* cmd, const char* package_dir) {
    dbg(2, "Executing special command: %s");
    char* special_cmd = calloc(64 + strlen(package_dir) + strlen(cmd), sizeof(char));

    if (system(special_cmd) != 0) {
        free(special_cmd);
        return 1;
    }
    free(special_cmd);
    dbg(1, "Special command executed!");
    return 0;
}
