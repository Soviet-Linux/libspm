#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/stat.h>
#include <unistd.h>
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
    (void)build_dir;
    char* make_dir = getenv("SOVIET_MAKE_DIR");

    char* cmd_params;
    if (QUIET) {
        cmd_params = "&> /dev/null";
    } else {
        cmd_params = "";
    }

    // TODO: this
    // Thinking about putting the package caching here
    // Maybe it will check if the installed version matches $VERSION
    // If so, it will just copy the dir from /usr/src/$NAME-$VERSION
    // Instead of executing the following:
    //
    // Parse the files
    for (int i = 0; i < pkg->filesCount; i++)
    {
        int download_attempts = 3;
        (void)download_attempts;
	int download_success = 0;
	(void)download_success;

        struct stat st_source = {0};
        struct stat st_source_loc = {0};

        char* location = calloc(MAX_PATH, 1);
        char* source_location = calloc(MAX_PATH, 1);
        char* source_file_location = calloc(MAX_PATH, 1);

        // This seems stupid, but should work
        char* file_name = strtok(pkg->files[i], " ");
        parse_env(&file_name);
        char* file_url = strtok(NULL, " ");
        parse_env(&file_url);
        char* file_sha256 = strtok(NULL, " ");

        sprintf(location, "%s/%s", getenv("SOVIET_MAKE_DIR"), file_name);
        sprintf(source_location, "%s/%s-%s", getenv("SOVIET_SOURCE_DIR"), getenv("NAME"), getenv("VERSION"));
        sprintf(source_file_location, "%s/%s-%s/%s", getenv("SOVIET_SOURCE_DIR"), getenv("NAME"), getenv("VERSION"), file_name);

        dbg(1, "Downloading %s", file_name);

        if (stat(source_location, &st_source_loc) == -1) 
        {
            mkdir(source_location, 0755);
            chown(source_location, getuid(), getgid());
            chmod(source_location, 0755);
        }


        if (stat(source_file_location, &st_source) == -1)
        {
            FILE* fp = fopen(location, "wb");
            download(file_url, fp);
            fclose(fp);

            // Check if the checksum shall be bypassed
          
            if (INSECURE) {
                msg(WARNING, "The Checksum is being skipped");
                goto skip_checksum;
            }

            // Check the hash, abort if mismatch
            unsigned char hash[SHA256_DIGEST_LENGTH];
            char* hash_str = calloc(SHA256_DIGEST_LENGTH, 8);

            struct stat st;
            stat(location, &st);
            int size = st.st_size;

            char* buffer = malloc(size);
            FILE *ptr;
            ptr = fopen(location,"r"); 
            fread(buffer, sizeof(char), size, ptr); 

            if (buffer == NULL) {
                    msg(FATAL, "Could not verify the file's hash");
                    return -1;
            }

            SHA256((unsigned char*) buffer, size, hash);

	   /* This caused and warning and functionally does nothing. Here hash is an array of unsigned char, but arrays in C are not pointers that can be NULL. This should probably be done with fread or fopen instead. Commenting out for now to silence the warning*/
            /*if (hash == NULL) {
                    msg(FATAL, "Could not verify the file's hash");
                    return -1;
            }*/
            
            dbg(1, "Hash is %s", file_sha256);
            for(int k = 0; k < SHA256_DIGEST_LENGTH; k++) {
                char* temp = calloc(8, 1);
                sprintf(temp, "%02x", hash[k]);
                strcat(hash_str, temp);
            }

            dbg(1, "Got %s", hash_str);
            if(strcmp(hash_str, file_sha256) != 0) {
                msg(FATAL, "Hash mismatch, aborting");
            }
            free(hash_str);
            free(buffer);

            skip_checksum:

            dbg(1, "Download finished");

            loadFile(location, source_file_location);            
        }
        else {
            dbg(1, "Loading form %s", source_location);
            loadFile(source_file_location, location);
        }

        free(location);
        free(source_location);
        free(source_file_location);
    }

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

    // Run 'prepare' command
    if (pkg->info.prepare != NULL && strlen(pkg->info.prepare) > 0) {
        char prepare_cmd[64 + strlen(package_dir) + strlen(pkg->info.prepare) + strlen(cmd_params)];

        sprintf(prepare_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.prepare, cmd_params);

        dbg(2, "Executing prepare command: %s", prepare_cmd);
        if (system(prepare_cmd) != 0) {
            msg(FATAL, "Failed to prepare %s", pkg->name);
            return -2;
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
    dbg(2, "Executing special command: %s", cmd);

    if (system(cmd) != 0) {
        return 1;
    }
    dbg(1, "Special command executed!");
    return 0;
}
