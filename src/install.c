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
        
        if (make_dir == NULL || build_dir == NULL)
        {
            msg(ERROR, "SOVIET_MAKE_DIR or SOVIET_BUILD_DIR is not set");
            return -1;
        }
    }

    msg(INFO, "Installing %s", pkg->name);

    // Set the package info section as environment vadiables for make script
    {
        dbg(2, "%s", pkg->name);
        setenv("NAME", pkg->name, 1);
        dbg(2, "%s", pkg->version);
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
    }

    // Check if a package is a collection
    if(strcmp(pkg->type, "con") != 0)
    {
        // Legacy directory path for compatibility
        char legacy_dir[MAX_PATH];
        sprintf(legacy_dir, "%s/%s-%s", getenv("SOVIET_MAKE_DIR"), pkg->name, pkg->version);

        //  Build the package
        {
            // ...
            chmod(getenv("SOVIET_MAKE_DIR"), 0777);
            chmod(getenv("SOVIET_BUILD_DIR"), 0777);

            pid_t p = fork(); 
            int status = 0;
            if ( p == 0)
            {
                // Ensure that the build is done as a regular user
                {
                    if (getuid() == 0) 
                    {
                        // process is running as root, drop privileges
                        if (setgid(65534) != 0)
                        {
                            msg(ERROR, "setgid: Unable to drop group privileges");
                        }
                        if (setuid(65534) != 0)
                        {
                            msg(ERROR, "setuid: Unable to drop user privileges");
                        }
                    }
                }
                
                // Build the package
                dbg(1, "Making %s", pkg->name);
                make(pkg);
                exit(0);
            } 
            else
            {
                waitpid(p, &status, 0);
            }

            if(WIFSIGNALED(status))
            {
                msg(FATAL, "WIFSIGNALED");
            }
            if(WEXITSTATUS(status) != 0)
            {
                msg(FATAL, "make exited with error code %d", WEXITSTATUS(status));
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
            if (check(pkg) == 0) {
                msg(WARNING, "Package %s is already installed, reinstalling", pkg->name);
                uninstall(pkg);
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
            dbg(1, "Executing post install script for %s", pkg->name);
            if (system(pkg->info.special) != 0) 
            {
                msg(FATAL, "Failed to run post-install script for %s", pkg->name);
                return -2;
            }
        }
    }

    if(create_pkg(getenv("SOVIET_SPM_DIR"), pkg) != 0) return 1;

    dbg(1, "Package %s installed", pkg->name);
    
    // Clean up
    clean();
    return 0;
}

/* Warning: there is something sussy going on beyond this point */
void write_package_configuration_file(struct package* pkg)
{
    // Set global environment variables
    if (pkg->config != NULL && pkg->configCount > 0 && strlen(pkg->config[0]) > 0) 
    {
        dbg(1, "Setting environment variables...");
        char* env_path = calloc(MAX_PATH, 1);
        sprintf(env_path, "%s/%s", getenv("SOVIET_ENV_DIR"), pkg->name);

        FILE *env_file;
        env_file = fopen(env_path, "w"); 

        for (int i = 0; i < pkg->configCount; i++)
        {
            fprintf(env_file, "%s\n", pkg->config[i]);
        }
        fclose(env_file);
        free(env_path);
    }
}

void read_package_configuration_file(struct package* pkg)
{
    // Get global environment variables
    if (pkg->environment != NULL) 
    {
        dbg(1, "Getting environment variables...");
        char* env_path = calloc(MAX_PATH, 1);
        sprintf(env_path, "%s/%s", getenv("SOVIET_ENV_DIR"), pkg->environment);

        readConfig(env_path, 1);
        free(env_path);
    }


    // Set global environment variables
    if (pkg->config != NULL && pkg->configCount > 0 && strlen(pkg->config[0]) > 0) 
    {
        dbg(1, "Setting environment variables...");
        char* env_path = calloc(MAX_PATH, 1);
        sprintf(env_path, "%s/%s", getenv("SOVIET_ENV_DIR"), pkg->name);

        readConfig(env_path, 1);
        free(env_path);
    }
}