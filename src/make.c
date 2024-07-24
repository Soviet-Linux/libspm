#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/stat.h>
#include <openssl/sha.h>

// Include necessary headers
#include "libspm.h"
#include "cutils.h"
#include "globals.h"



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

// Function to download package sources with retries
/*
Accepts:
- const char* make_dir: Path to the make directory.
- const char* download_command: Command to download the package.
- const char* cmd_params: Additional command parameters.
- int retries: Number of times to retry the download if the hash does not match.

Returns:
- int: An integer indicating the result of the download process.
  - 0: Download succeeded.
  - -1: Failed to download sources.
*/
int download_package_sources(const char* make_dir, const char* download_command, const char* cmd_params, int retries) {
    int res = -1;
    for (int attempt = 0; attempt < retries; attempt++) {
        if (download_command != NULL && strlen(download_command) > 0) {
            char sources_cmd[64 + strlen(make_dir) + strlen(download_command)];

            sprintf(sources_cmd, "(cd %s && %s) %s ", make_dir, download_command, cmd_params);
            dbg(2, "Downloading sources with %s (Attempt %d)", sources_cmd, attempt + 1);
            res = system(sources_cmd);

            if (res == 0) {
                break;
            }
        }
        msg(WARNING, "Failed to download sources, retrying (%d/%d)...", attempt + 1, retries);
    }

    if (res != 0) {
        msg(ERROR, "Failed to download sources after %d attempts.", retries);
        return -1;
    }

    return 0;
}

// Function to calculate SHA256 hash of a file
/*
Accepts:
- const char* filename: Path to the file whose hash needs to be calculated.

Returns:
- const char*: A dynamically allocated string containing the SHA256 hash in hexadecimal format.
         It's the responsibility of the caller to free this memory.
*/
const char* calculate_hash(const char* filename) {
    FILE *ptr;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char* hash_str = NULL;

    ptr = fopen(filename, "rb");
    if (ptr == NULL) {
        msg(FATAL, "Could not open file %s for hash calculation", filename);
        return NULL;
    }

    struct stat st;
    stat(filename, &st);
    int size = st.st_size;

    unsigned char* buffer = malloc(size);  // Changed buffer to unsigned char*
    if (buffer == NULL) {
        fclose(ptr);
        msg(FATAL, "Memory allocation failed for buffer in calculate_hash");
        return NULL;
    }

    fread(buffer, sizeof(unsigned char), size, ptr);  // Updated sizeof to unsigned char
    fclose(ptr);

    SHA256(buffer, size, hash);
    hash_str = malloc((SHA256_DIGEST_LENGTH * 2 + 1) * sizeof(char));

    if (hash_str == NULL) {
        free(buffer);
        msg(FATAL, "Memory allocation failed for hash_str in calculate_hash");
        return NULL;
    }

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_str + (i * 2), "%02x", hash[i]);
    }

    free(buffer);
    return hash_str;
}


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
    for (int i = 0; i < pkg->inputsCount; i++) {
        msg(INFO, "%s", pkg->inputs[i]);
        char* str = calloc(MAX_PATH, sizeof(char));

        if (!OVERWRITE_CHOISE) {
            char* res = fgets(str, MAX_PATH - 1, stdin);

            if (strchr(str, '\n') == NULL) {
                while ((getchar()) != '\n');
            }

            int k = 0;
            while (str[k] != '\n' && str[k] != '\0') {
                if (strstr(str + k, "~`#$&*()\\|[]{};\'<>?!")) {
                    str[k] = ' ';
                }
                k++;
            }

            if (str[k] == '\n') {
                str[k] = '\0';
            }

            char* in = calloc(128, sizeof(char));
            sprintf(in, "INPUT_%d", i);
            setenv(in, str, 0);
            free(in);
        } else {
            sprintf(str, "%s", USER_CHOISE[0]);
            char* in = calloc(128, sizeof(char));
            sprintf(in, "INPUT_%d", i);
            setenv(in, str, 0);
            free(in);
        }
        free(str);
    }

    // Extract URL from the package
    char excmd[64 + strlen(pkg->url)];
    sprintf(excmd, "echo -n %s", pkg->url);
    char* exurl = exec(excmd);
    setenv("URL", exurl, 1);
    free(exurl);

    // Download package sources and check hash
    int download_attempt = 0;
    char* hash_str = NULL;

    while (download_attempt < 3) {
        if (download_package_sources(make_dir, pkg->info.download, cmd_params, 1) == 0) {
            // Calculate the hash of the downloaded file
            char* exec_cmd_1 = calloc(MAX_PATH, sizeof(char));
            sprintf(exec_cmd_1, "( cd %s && find . -maxdepth 1 -type f  | cut -c3- ) ", getenv("SOVIET_MAKE_DIR"));
            dbg(1, "Executing %s to search for package tarball", exec_cmd_1);
            char* file = exec(exec_cmd_1);
            file[strcspn(file, "\n")] = 0;

            char* filename = calloc(MAX_PATH, sizeof(char));
            sprintf(filename, "%s/%s", getenv("SOVIET_MAKE_DIR"), file);

            dbg(1, "Checking file %s for hash calculation", file);
            hash_str = calculate_hash(filename);
            free(exec_cmd_1);
            free(filename);

            // Compare hashes
            if (hash_str != NULL && strcmp(hash_str, pkg->sha256) == 0) {
                break;  // Hash matches, break out of loop
            } else {
                if (hash_str != NULL) {
                    free(hash_str);
                    hash_str = NULL;
                }
                msg(WARNING, "Hash mismatch, retrying download and hash check...");
                download_attempt++;
            }
        } else {
            msg(WARNING, "Failed to download sources, retrying...");
            download_attempt++;
        }
    }

    if (download_attempt >= 3) {
        msg(FATAL, "Failed to download sources after 3 attempts.");
        return -1;
    }

    if (hash_str == NULL) {
        msg(FATAL, "Failed to calculate hash of downloaded file.");
        return -1;
    }

    // Run 'prepare' command
    if (pkg->info.prepare != NULL && strlen(pkg->info.prepare) > 0) {
        char prepare_cmd[64 + strlen(package_dir) + strlen(pkg->info.prepare) + strlen(cmd_params)];

        sprintf(prepare_cmd, "( cd %s && %s ) %s", package_dir, pkg->info.prepare, cmd_params);

        dbg(2, "Executing prepare command: %s", prepare_cmd);
        if (system(prepare_cmd) != 0) {
            free(hash_str);
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
            free(hash_str);
            return 1;
        }
        dbg(1, "Make command executed!");
    }

    // Run 'test' command (if in testing mode)
    // TODO: Implement test command execution logic if needed

    free(hash_str);
    return 0;
}