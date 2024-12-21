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

// Function to install a package from source with a specific format
/*
Accepts:
- const char* spm_path: Path to the package archive.

Returns:
- int: An integer indicating the result of the installation.
  - 0: Package installed successfully.
  - -1: Installation failed.
*/
int install_package_source(struct package* pkg) 
{
    // Get required directory paths
    {
        char* make_dir = getenv("SOVIET_MAKE_DIR");
        char* build_dir = getenv("SOVIET_BUILD_DIR");
        char* format = getenv("SOVIET_DEFAULT_FORMAT");
        // Temporary for compatibility
        setenv("BUILD_ROOT", build_dir, 1);

        if (make_dir == NULL || build_dir == NULL)
        {
            msg(ERROR, "SOVIET_MAKE_DIR or SOVIET_BUILD_DIR is not set");
            return -1;
        }
    }

    msg(INFO, "Installing %s", pkg->name);

    // Set the package info section as environment vadiables for make script
    {
        setenv("NAME", pkg->name, 1);
        setenv("VERSION", pkg->version, 1);

        if (pkg->url != NULL)
        {       
            parse_env(&(pkg->url));
            dbg(1, "URL: %s", pkg->url);
            setenv("URL", pkg->url, 1);
        }

        if (pkg->type != NULL)
        {
            setenv("TYPE", pkg->type, 1);
        }

        if (pkg->license != NULL)
        {
            setenv("LICENSE", pkg->license, 1);
        }

        if (pkg->sha256 != NULL)
        {
            setenv("SHA256", pkg->sha256, 1);
        }
    }

    // Check if a package is a collection
    if(strcmp(pkg->type, "con") != 0)
    {
        // Legacy directory path for compatibility
        char legacy_dir[MAX_PATH];

        //  Build the package
        {
            sprintf(legacy_dir, "%s/%s-%s", getenv("SOVIET_MAKE_DIR"), pkg->name, pkg->version);

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
                dbg(1, "Making %s", pkg->name);
                if (make(legacy_dir, &pkg) != 0) {
                    msg(ERROR, "Failed to make %s", pkg->name);
                    exit(1);
                }
                exit(0);
            } 
            while(wait(&status) > 0);

            if(WEXITSTATUS(status) != 0)
            {
                msg(FATAL, "make exited with error code %d", pkg->name, WEXITSTATUS(status));
            }

            dbg(1, "Making %s done", pkg->name);
        }

        // Run 'install' command
        {
            if (pkg->info.install == NULL && strlen(pkg->info.install) == 0) {
                msg(FATAL, "No install command!");
            }

            char install_cmd[64 + strlen(legacy_dir) + strlen(pkg->info.install)];
            sprintf(install_cmd, "( cd %s && %s )", legacy_dir, pkg->info.install);

            dbg(2, "Executing install command: %s", install_cmd);
            if (system(install_cmd) != 0) {
                msg(FATAL, "Failed to install %s", pkg->name);
                return -2;
            }
            clean_install();
            dbg(1, "Install command executed!");
        }

        // Get package locations
        {
            dbg(1, "Getting locations for %s", pkg->name);
            pkg->locations = get_all_files(getenv("SOVIET_BUILD_DIR") + 1, getenv("SOVIET_BUILD_DIR"), &(pkg->locationsCount));
            
            if (pkg->locationsCount <= 0) {
                msg(ERROR, "Failed to get locations for %s", pkg->name);
                return -1;
            }

            dbg(1, "Got %d locations for %s", pkg->locationsCount, pkg->name);

            // Check if the package is already installed
            if (is_installed(pkg->name)) {
                msg(WARNING, "Package %s is already installed, reinstalling", pkg->name);
                uninstall(pkg->name);
            } else {
                dbg(3, "Package %s is not installed", pkg->name);
            }

            // Move binaries to their destination
            dbg(1, "Moving binaries for %s", pkg->name);
            move_binaries(pkg->locations, pkg->locationsCount);
        }
    }

    // Execute post-install scripts
    {
        if (pkg->info.special != NULL && strlen(pkg->info.special) > 0) 
        {
            msg(WARNING, "Special: %s", pkg->info.special);
            dbg(1, "Executing post install script for %s", pkg->name);
            if (system(pkg->info.special) != 0) 
            {
                msg(FATAL, "Failed to run post-install script for %s", pkg->name);
                return -2;
            }
        }
    }

    create_pkg(&pkg, NULL); 
    dbg(1, "Package %s installed", pkg->name);

    // Clean up
    clean();

    // Remove the package from the queue
    QUEUE_COUNT--;
    PACKAGE_QUEUE[QUEUE_COUNT] = NULL;

    return 0;
}

/* Warning: there is something sussy going on beyond this point */
int configure_package(struct package* pkg)
{
    // Get global environment variables
    if (pkg->environment != NULL) 
    {
        dbg(1, "Getting environment variables...");
        char* env_path = calloc(MAX_PATH, 1);
        sprintf(env_path, "%s/%s", getenv("SOVIET_ENV_DIR"), pkg->environment);

        readConfig(env_path, 1);
    }

    // Set global environment variables
    if (pkg->exports != NULL && pkg->exportsCount > 0 && strlen(pkg->exports[0]) > 0) 
    {
        dbg(1, "Setting environment variables...");
        char* env_path = calloc(MAX_PATH, 1);
        sprintf(env_path, "%s/%s", getenv("SOVIET_ENV_DIR"), pkg->name);

        FILE *env_file;
        env_file = fopen(env_path, "w"); 

        for (int i = 0; i < pkg->exportsCount; i++)
        {
            fprintf(env_file, "%s\n", pkg->exports[i]);
            char* line = strdup(pkg->exports[i]);
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
bool is_installed(const char* name)
{
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

int add_to_queue(const char* name)
{
    // Add the package name to the queue
    PACKAGE_QUEUE[QUEUE_COUNT] = name;
    QUEUE_COUNT++;
    if (QUEUE_COUNT > QUEUE_MAX)
    {
        msg(FATAL, "Package tree too large");
    }

    dbg(1, "Added %s to the queue", name);
}