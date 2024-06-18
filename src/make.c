#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/stat.h>
#include <openssl/sha.h>

// Include necessary headers
#include "libspm.h"
#include "cutils.h"
#include "globals.h"

// Function to build and install a package
/*
Accepts:
- char* package_dir: Path to the package directory.
- struct package* pkg: Pointer to the package structure.

Returns:
- int: An integer indicating the result of the build and installation process.
  - 0: Build and installation succeeded.
  - 1: An error occurred during execution of prepare, make, test, or install commands.
  - -2: Failed to install the package.
  - -3: No install command found.
*/
int make(char* package_dir, struct package* pkg) {
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

    // Get user input

    for (int i = 0; i < pkg->inputsCount; i++) 
    {
        msg(INFO, "%s", pkg->inputs[i]);
        char* str = calloc(MAX_PATH, sizeof(char));

        if(OVERWRITE_CHOISE != true)
        {
            char* res = fgets(str, MAX_PATH-1, stdin);

            if ( strchr(str, '\n') == NULL )
            {
                while ((getchar()) != '\n');
            }

            int k = 0;

            while (str[k] != '\n' && str[k] != '\0')
            {
                if(strstr(str[k], "~`#$&*()\\|[]{};\'<>?!"))
                {
                    str[k] = ' ';
                }
                k++;
            }

            if (str[k] == '\n')
            {
                str[k] = '\0';
            }

            char* in = calloc(128, sizeof(char));
            sprintf("%s", in, "INPUT_%d", i);
            setenv(in, str, 0);
            free(in);
        }
            else
            {
                if(sizeof(USER_CHOISE[0]) <= sizeof(str))
                {
                    sprintf(str, "%s", USER_CHOISE[0]);
                    char* in = calloc(128, sizeof(char));
                    sprintf(in, "INPUT_%d", i);
                    setenv(in, str, 0);
                    free(in);
                }
                    else
                    {
                        msg(FATAL, "something somwhere went wrong");
                    }
            }
        free(str);
    }

    // Thinking about putting the package caching here
    // Maybe it will check if the installed version matches $VERSION
    // If so, it will just copy the dir from /usr/src/$NAME-$VERSION
    // Instead of executing the following:
    //
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
    if (!INSECURE)
    {
        // Check the hash, abort if mismatch
        char* exec_cmd_1 = calloc(MAX_PATH, sizeof(char));
        unsigned char hash[SHA256_DIGEST_LENGTH];
        char* hash_str = calloc(SHA256_DIGEST_LENGTH + 1, 1);

        // TODO, fix this
        sprintf(exec_cmd_1, "( cd %s && find . -maxdepth 1 -type f  | cut -c3- ) ", getenv("SOVIET_MAKE_DIR"));
        dbg(1, "Executing  %s to search for package tarball", exec_cmd_1);
        char* file = exec(exec_cmd_1);
        file[strcspn(file, "\n")] = 0;

        char* filename = calloc(MAX_PATH, sizeof(char));
        sprintf(filename, "%s/%s", getenv("SOVIET_MAKE_DIR"), file);

        dbg(1, "Checking file %s, if this is not the package tarball, the author of this line is stupid", file);
        struct stat st;
        stat(filename, &st);
        int size = st.st_size;

        char* buffer = malloc(size);
        FILE *ptr;
        ptr = fopen(filename,"r"); 
        fread(buffer, sizeof(char), size, ptr); 

        if (!(buffer == NULL))
        {
            SHA256(buffer, size, hash);

            if (!((hash == NULL) || (hash[0] == '\0')))
            {
                dbg(1, "Hash is %s", pkg->sha256);
                for(int k = 0; k < SHA256_DIGEST_LENGTH; k++)
                {
                    char* temp = calloc(1, 1);
                    sprintf(temp, "%02x", hash[k]);
                    strcat(hash_str, temp);
                }

                dbg(1, "Got %s", hash_str);
                if(strcmp(hash_str, pkg->sha256) != 0)
                {
                    msg(FATAL, "Hash mismatch, aborting");
                }
            }
        }
            else
            {
                msg(FATAL, "Could not verify the file's hash");
            }

            free(exec_cmd_1);
    } 
        else 
        {
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
    char* special_cmd = calloc(64 + strlen(package_dir) + strlen(cmd), sizeof(char));
    dbg(2, "Executing special command: %s", cmd);

    if (system(cmd) != 0) {
        return 1;
    }
    dbg(1, "Special command executed!");
    return 0;
}
