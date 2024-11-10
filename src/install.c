#define _GNU_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Include necessary headers
#include "libspm.h"
#include "cutils.h"

// include header for wait()
#include <sys/wait.h>

// Function to install a package from source (archive)
/*
Accepts:
- const char* spm_path: Path to the package archive.
- int as_dep: Flag indicating if the package is a dependency.

Returns:
- int: An integer indicating the result of the installation.
  - 0: Package installed successfully.
  - -1: Installation failed.
*/
int install_package_source(const char* spm_path, int as_dep) {
    return f_install_package_source(spm_path, as_dep, NULL);
}

// Function to install a package from source with a specific format
/*
Accepts:
- const char* spm_path: Path to the package archive.
- int as_dep: Flag indicating if the package is a dependency.
- const char* format: Specific package format (optional).

Returns:
- int: An integer indicating the result of the installation.
  - 0: Package installed successfully.
  - -1: Installation failed.
*/
int f_install_package_source(const char* spm_path, int as_dep, char* repo) {
    // Check if spm_path is NULL

    if (spm_path == NULL) {
        msg(ERROR, "spm_path is NULL");
        return -1;
    }

    // Get required directory paths
    char* make_dir = getenv("SOVIET_MAKE_DIR");
    char* build_dir = getenv("SOVIET_BUILD_DIR");

    if (make_dir == NULL || build_dir == NULL) {
        msg(ERROR, "SOVIET_MAKE_DIR or SOVIET_BUILD_DIR is not set");
        return -1;
    }

    // Initialize the package structure
    struct package pkg = {0};

    char* format = "ecmp";

    // Attempt to open the package archive
    if (open_pkg(spm_path, &pkg, format) != 0) {
        msg(ERROR, "Failed to open package");
        return -1;
    }

    msg(INFO, "Installing %s", pkg.name);

    // Add the package name to the queue
    PACKAGE_QUEUE[QUEUE_COUNT] = pkg.name;
    QUEUE_COUNT++;
    dbg(1, "Added %s to the queue", pkg.name);

    // Set the package info section as environment vadiables for make script
    setenv("NAME", pkg.name, 1);
    setenv("VERSION", pkg.version, 1);

    if (pkg.url != NULL)
    {       
        parse_env(&(pkg.url));
        dbg(1, "URL: %s", pkg.url);
        setenv("URL", pkg.url, 1);
    }

    if (pkg.type != NULL)
    {
        setenv("TYPE", pkg.type, 1);
    }

    if (pkg.license != NULL)
    {
        setenv("LICENSE", pkg.license, 1);
    }

    if (pkg.sha256 != NULL)
    {
        setenv("SHA256", pkg.sha256, 1);
    }

    // Set environment variables for building
    setenv("BUILD_ROOT", build_dir, 1);

    /* Warning: there is something sussy going on beyond this point */

    // Get global environment variables
    if (pkg.environment != NULL) 
    {
        dbg(1, "Getting environment variables...");
        char* env_path = calloc(MAX_PATH, 1);
        sprintf(env_path, "%s/%s", getenv("SOVIET_ENV_DIR"), pkg.environment);

        readConfig(env_path, 1);
    }

    // Set global environment variables
    if (pkg.exports != NULL && pkg.exportsCount > 0 && strlen(pkg.exports[0]) > 0) 
    {
        dbg(1, "Setting environment variables...");
        char* env_path = calloc(MAX_PATH, 1);
        sprintf(env_path, "%s/%s", getenv("SOVIET_ENV_DIR"), pkg.name);

        FILE *env_file;
        env_file = fopen(env_path, "w"); 

        for (int i = 0; i < pkg.exportsCount; i++)
        {
            fprintf(env_file, "%s\n", pkg.exports[i]);
            char* line = strdup(pkg.exports[i]);
            parse_env(&line);

            if(((line[0] != '#') && ((line[0] != '/') && (line[1] != '/'))) && (strstr(line, "=") != 0))
            {
                char* key = strtok(line, "=");
                char* value = strchr(line, '\0') + 1;

                if (key == NULL || value == NULL) 
                {
                    msg(ERROR, "Invalid config file");
                }

                dbg(2, "Key: %s Value: %s", key, value);

                // Set environment variables based on the key-value pairs in the config file
                setenv(key, value, 1);
            }
            free(line);
        }

        fclose(env_file);
    }


    // Check if a package is a collection
    if(strcmp(pkg.type, "con") != 0)
    {
        // Legacy directory path for compatibility
        char legacy_dir[MAX_PATH];
        sprintf(legacy_dir, "%s/%s-%s", getenv("SOVIET_MAKE_DIR"), pkg.name, pkg.version);

        // ...
        chmod(getenv("SOVIET_MAKE_DIR"), 0777);
        chmod(getenv("SOVIET_BUILD_DIR"), 0777);

        pid_t p = fork(); 
        int status = 0;
        if ( p == 0)
        {

            if (getuid() == 0) 
            {
                /* process is running as root, drop privileges */
                if (setgid(65534) != 0)
                {
                    msg(ERROR, "setgid: Unable to drop group privileges");
                }
                if (setuid(65534) != 0)
                {
                    msg(ERROR, "setuid: Unable to drop user privileges");
                }
            }
            // Build the package
            dbg(1, "Making %s", pkg.name);
            if (make(legacy_dir, &pkg) != 0) {
                msg(ERROR, "Failed to make %s", pkg.name);
                exit(1);
            }
            exit(0);
        } 
        while(wait(&status) > 0);

        if(WEXITSTATUS(status) != 0)
        {
            msg(FATAL, "make exited with error code %d", pkg.name, status);
        }

        dbg(1, "Making %s done", pkg.name);

        // Run 'install' command
        if (pkg.info.install == NULL && strlen(pkg.info.install) == 0) {
            msg(FATAL, "No install command!");
        }

        char install_cmd[64 + strlen(legacy_dir) + strlen(pkg.info.install)];
        sprintf(install_cmd, "( cd %s && %s )", legacy_dir, pkg.info.install);

        dbg(2, "Executing install command: %s", install_cmd);
        if (system(install_cmd) != 0) {
            msg(FATAL, "Failed to install %s", pkg.name);
            return -2;
        }
        clean_install();
        dbg(1, "Install command executed!");


        // Get package locations
        dbg(1, "Getting locations for %s", pkg.name);
        pkg.locationsCount = get_locations(&pkg.locations, getenv("SOVIET_BUILD_DIR"));
        
        if (pkg.locationsCount <= 0) {
            msg(ERROR, "Failed to get locations for %s", pkg.name);
            return -1;
        }

        dbg(1, "Got %d locations for %s", pkg.locationsCount, pkg.name);

        // Check if the package is already installed
        if (is_installed(pkg.name)) {
            msg(WARNING, "Package %s is already installed, reinstalling", pkg.name);
            uninstall(pkg.name);
        } else {
            dbg(3, "Package %s is not installed", pkg.name);
        }

        // Move binaries to their destination
        dbg(1, "Moving binaries for %s", pkg.name);
        move_binaries(pkg.locations, pkg.locationsCount);
    }

    // Execute post-install scripts
    if (pkg.info.special != NULL && strlen(pkg.info.special) > 0) {
        msg(WARNING, "Special: %s", pkg.info.special);
        dbg(1, "Executing post install script for %s", pkg.name);
        exec_special(pkg.info.special, getenv("SOVIET_BUILD_DIR"));
    }

    // Format the path using sprintf
    char file_path[MAX_PATH];

    if(!repo)
    {
        repo = "local";
    }

    dbg(1, "spm dir is %s", getenv("SOVIET_SPM_DIR"));
    dbg(1, "repo is %s", repo);
    dbg(1, "name is %s", pkg.name);
    dbg(1, "description is %s", pkg.info.description);


    char repo_path[MAX_PATH];
    sprintf(repo_path, "%s/%s", getenv("SOVIET_SPM_DIR"), repo);

    if(isdir(repo_path) != 0)
    {
        pmkdir(repo_path);
    }

    sprintf(file_path, "%s/%s/%s.%s", getenv("SOVIET_SPM_DIR"), repo, pkg.name, getenv("SOVIET_DEFAULT_FORMAT"));
    create_pkg(file_path, &pkg, NULL); 
    dbg(1, "Package %s installed", pkg.name);

    // Clean up
    clean();

    // Remove the package from the queue
    QUEUE_COUNT--;
    PACKAGE_QUEUE[QUEUE_COUNT] = NULL;

    // Free allocated memory
    free_pkg(&pkg);
    return 0;
}

// Utilities declaration for binary install
// Untar a binary package to another directory
/*
Accepts:
- const char* bin_path: Path to the binary package archive.
- const char* dest_dir: Destination directory for untarring.

Returns:
- int: An integer indicating the result of the untarring.
  - 0: Untarring completed successfully.
  - Non-zero: An error occurred during untarring.
*/
__attribute__((unused)) int uncompress_binary(const char* bin_path, const char* dest_dir);

// Function to install a package from a binary archive
/*
Accepts:
- const char* archivePath: Path to the binary archive.
- int as_dep: Flag indicating if the package is a dependency.

Returns:
- int: An integer indicating the result of the installation.
  - 0: Package installed successfully.
  - -1: Installation failed.
*/
int install_package_binary(const char* archivePath, int as_dep, const char* repo) {

    struct package pkg;

    // Get required environment variables
    char* default_format = getenv("SOVIET_DEFAULT_FORMAT");
    char* build_dir = getenv("SOVIET_BUILD_DIR");
    char* spm_dir = getenv("SOVIET_SPM_DIR");

    if (default_format == NULL || build_dir == NULL || spm_dir == NULL) {
        msg(ERROR, "Environment variables not set");
        return -1;
    }

    // Initialize package name
    pkg.name = calloc(sizeof(archivePath), sizeof(char));

    // Get the package name from the binary archive
    if (get_bin_name(archivePath, pkg.name) != 0) {
        msg(ERROR, "Could not get name from archive path");
        return -1;
    }

    // Uncompress binary and check the output
    if (uncompress_binary(archivePath, build_dir) != 0)
        return -1;

    // Format the path using sprintf
    char spm_path[MAX_PATH];
    sprintf(spm_path, "%s/%s.%s", build_dir, pkg.name, default_format);
    if (access(spm_path, F_OK) != 0) {
        msg(ERROR, "%s not found", spm_path);
        return -1;
    }

    // Open the package
    open_pkg(spm_path, &pkg, NULL);

    // Add the package name to the queue
    PACKAGE_QUEUE[QUEUE_COUNT] = pkg.name;
    QUEUE_COUNT++;
    dbg(1, "Added %s to QUEUE[%d]", pkg.name, QUEUE_COUNT - 1);

    // Move binaries to their destination
    dbg(1, "Moving binaries for %s", pkg.name);
    move_binaries(pkg.locations, pkg.locationsCount);

    // Execute post-install scripts
    exec_special(pkg.info.special, build_dir);

    // Format the path using sprintf
    char file_path[MAX_PATH];
    sprintf(file_path, "%s/%s/%s.%s", getenv("SOVIET_SPM_DIR"), repo, pkg.name, getenv("SOVIET_DEFAULT_FORMAT"));
    create_pkg(file_path, &pkg, NULL);

    dbg(1, "Package %s installed", pkg.name);

    free_pkg(&pkg);

    // Clean up
    clean();

    // Remove the package from the queue
    QUEUE_COUNT--;
    PACKAGE_QUEUE[QUEUE_COUNT] = NULL;

    return 0;
}

// Function to check if a package is already installed
/*
Accepts:
- const char* name: Name of the package to check.

Returns:
- bool: A boolean value indicating whether the package is installed.
  - true: Package is installed.
  - false: Package is not installed.
*/
bool is_installed(const char* name) {
    char path[1024];
    char** FORMATS;
    int FORMAT_COUNT = splita(strdup(getenv("SOVIET_FORMATS")),' ',&FORMATS);

    char** REPOS = calloc(512,sizeof(char));
    int REPO_COUNT = get_repos(REPOS);

    // loop through all formats
    for (int i = 0; i < FORMAT_COUNT; i++)
    {
        // loop through all repos
        for (int j = 0; j < REPO_COUNT; j++)
        {
            sprintf(path,"%s/%s/%s.%s",getenv("SOVIET_SPM_DIR"), REPOS[j],name,FORMATS[i]);
            if (access(path,F_OK) == 0)
            {
                free(REPOS);
                free(FORMATS);
                return true;
            }
        }
    }
    return false;
}

// Function to get the package name from a binary archive path
/*
Accepts:
- const char* bin_path: Path to the binary package archive.
- char* name: A character array to store the package name.

Returns:
- int: An integer indicating the result of name extraction.
  - 0: Name extracted successfully.
  - -1: Extraction failed.
*/
int get_bin_name(const char* bin_path, char* name) {
    const char* file_name = strrchr(bin_path, '/');
    if (file_name == NULL)
        file_name = bin_path;
    else
        file_name++;
    for (int i = 0; i < (int)strlen(file_name); i++) {
        if (file_name[i] == '.') {
            sprintf(name, "%.*s", i, file_name);
            return 0;
        }
    }
    return -1;
}

// Function to free memory allocated for a package structure
/*
Accepts:
- struct package* pkg: Pointer to a package structure.

Returns:
- int: An integer indicating the result of memory deallocation.
  - 0: Memory freed successfully.
*/
int free_pkg(struct package* pkg) {
    if (pkg->name != NULL) free(pkg->name);
    if (pkg->version != NULL) free(pkg->version);
    if (pkg->license != NULL) free(pkg->license);
    if (pkg->type != NULL) free(pkg->type);
    if (pkg->url != NULL) free(pkg->url);

    if (pkg->info.make != NULL) free(pkg->info.make);
    if (pkg->info.special != NULL) free(pkg->info.special);
    if (pkg->info.download != NULL) free(pkg->info.download);
    if (pkg->info.install != NULL) free(pkg->info.install);
    if (pkg->info.prepare != NULL) free(pkg->info.prepare);
    if (pkg->info.test != NULL) free(pkg->info.test);

    if (pkg->locations) {
        if (*pkg->locations) free(*pkg->locations);
        free(pkg->locations);
    }
    if (pkg->dependencies) {
        if (*pkg->dependencies) free(*pkg->dependencies);
        free(pkg->dependencies);
    }
    if (pkg->optional) {
        if (*pkg->optional) free(*pkg->optional);
        free(pkg->optional);
    }
    if (pkg->files) {
        if (*pkg->files) free(*pkg->files);
        free(pkg->files);
    }
    return 0;
}

// Function to untar a binary package to a destination directory
/*
Accepts:
- const char* bin_path: Path to the binary package archive.
- const char* dest_dir: Destination directory for untarring.

Returns:
- int: An integer indicating the result of untarring.
  - 0: Untarring completed successfully.
  - Non-zero: An error occurred during untarring.
*/
int uncompress_binary(const char* bin_path, const char* dest_dir) {
    // Format the untar command using sprintf
    char untar_cmd[strlen(bin_path) + strlen(dest_dir) + 64];
    sprintf(untar_cmd, "tar -xvf %s -C %s", bin_path, dest_dir);

    // Execute the untar command
    return system(untar_cmd);
}
