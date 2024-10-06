#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Include necessary headers
#include "libspm.h"
#include "cutils.h"

//should probably add there to the header when we are done

int update()
{
    msg(INFO, "fetching updates");

    int new_version_found = 0;
    
    const char *path = getenv("SOVIET_SPM_DIR");
    dbg(2, "path is %s", path);
    const char *repo_path = getenv("SOVIET_REPOS_DIR");
    dbg(2, "repo path is %s", repo_path);
    int num_files;
    char **files_array = get_all_files(path, path, &num_files);

    if (files_array == NULL) {
        msg(WARNING, "no packages installed");
        return 0;
    }
    // Print each file path
    for (int i = 0; i < num_files; i++) 
    {
        // This will break if the files are not separated into repos
        // But it doesnt cause a crash, just a visual bug
        // I think
        char* local_repo = strtok(files_array[i], "/");
        //dbg(2, "local repo is %s", local_repo);
        char* local_package_name = strchr(files_array[i], '\0') + 1;
        //dbg(2, "local package name is %s", local_package_name);

        // Allocate the packages to be compared
        struct package* local = calloc(1, sizeof(struct package));
        struct package* remote = calloc(1, sizeof(struct package));

        char* local_path = calloc(MAX_PATH, sizeof(char));
        char* remote_path = calloc(MAX_PATH, sizeof(char));

        sprintf(local_path, "%s/%s/%s", path, local_repo, local_package_name);
        //dbg(2, "local path is %s", local_path);

        int num_searched_files;
        char **searched_files_array = get_all_files(repo_path, repo_path, &num_searched_files);

        if (searched_files_array != NULL) 
        {
            // Print each file path
            for (int j = 0; j < num_searched_files; j++) 
            {
                // This will break if the files are not separated into repos
                // But it doesnt cause a crash, just a visual bug
                // I think
                char* remote_repo = strtok(searched_files_array[j], "/");
                //dbg(2, "remote repo is %s", remote_repo);
                char* remote_package = strchr(searched_files_array[j], '\0') + 1;
                //dbg(2, "remote package is %s", remote_package);
                char* remote_package_name = calloc(strlen(remote_package) + 2, sizeof(char));
                strcpy(remote_package_name, remote_package);
                //dbg(2, "remote package name is %s", remote_package_name);

                while(strtok(remote_package_name, "/"))
                {
                    char* tmp = remote_package_name;
                    remote_package_name = strchr(remote_package_name, '\0') + 1;
                    if(strcmp(remote_package_name, "") == 0)
                    {
                        remote_package_name = tmp;
                        break;
                    }
                }
                
                //printf("%s, %s \n", remote_package_name, local_package_name);

                if (strcmp(remote_repo, local_repo) == 0)
                {
                    if (strcmp(remote_package_name, local_package_name) == 0)
                    {
                        // Compare the filename
                        sprintf(remote_path, "%s/%s/%s", repo_path, remote_repo, remote_package);
                        dbg(2, "remote path is %s", remote_path);

                        open_pkg(local_path, local, "ecmp");
                        open_pkg(remote_path, remote, "ecmp");

                        // Compare the versions
                        if(strcmp(local->version, remote->version) != 0)
                        {
                                msg(INFO, "package %s is at version %s, available version is %s", local->name, local->version, remote->version);
                                new_version_found = 1;
                        }

                        free(local);
                        free(remote);
                    }
                }
                // Free each file path string
                free(searched_files_array[j]);
            }
            // Free each file path string
            free(files_array[i]);
        }
        // Free the array of file paths
        free(searched_files_array);
    }
    // Free the array of file paths
    free(files_array);


    if(new_version_found != 0)
    {
        msg(WARNING, "new version found for one or more packages, use --upgrade to upgrade");
    }
        else
        {
            msg(WARNING, "all packages are up to date");
        }
    
    return 0;
}

int upgrade()
{
    msg(INFO, "upgrading");
    int new_version_installed = 0;

    const char *path = getenv("SOVIET_SPM_DIR");
    const char *repo_path = getenv("SOVIET_REPOS_DIR");
    int num_files;
    char **files_array = get_all_files(path, path, &num_files);

    if (files_array != NULL) 
    {
        // Print each file path
        for (int i = 0; i < num_files; i++) 
        {
            // This will break if the files are not separated into repos
            // But it doesnt cause a crash, just a visual bug
            // I think
            char* local_repo = strtok(files_array[i], "/");
            char* local_package_name = strchr(files_array[i], '\0') + 1;

            // Allocate the packages to be compared
            struct package* local = calloc(1, sizeof(struct package));
            struct package* remote = calloc(1, sizeof(struct package));

            char* local_path = calloc(MAX_PATH, sizeof(char));
            char* remote_path = calloc(MAX_PATH, sizeof(char));

            sprintf(local_path, "%s/%s/%s", path, local_repo, local_package_name);

            int num_searched_files;
            char **searched_files_array = get_all_files(repo_path, repo_path, &num_searched_files);

            if (searched_files_array != NULL) 
            {
                // Print each file path
                for (int j = 0; j < num_searched_files; j++) 
                {
                    // This will break if the files are not separated into repos
                    // But it doesnt cause a crash, just a visual bug
                    // I think
                    char* remote_repo = strtok(searched_files_array[j], "/");
                    char* remote_package = strchr(searched_files_array[j], '\0') + 1;
                    char* remote_package_name = calloc(strlen(remote_package) + 1, sizeof(char));
                    strcpy(remote_package_name, remote_package);

                    while(strtok(remote_package_name, "/"))
                    {
                        char* tmp = remote_package_name;
                        remote_package_name = strchr(remote_package_name, '\0') + 1;
                        if(strcmp(remote_package_name, "") == 0)
                        {
                            remote_package_name = tmp;
                            break;
                        }
                    }
                    
                    //printf("%s, %s \n", remote_package_name, local_package_name);

                    if (strcmp(remote_repo, local_repo) == 0)
                    {
                        if (strcmp(remote_package_name, local_package_name) == 0)
                        {
                            // Compare the filename
                            sprintf(remote_path, "%s/%s/%s", repo_path, remote_repo, remote_package);
                            
                            open_pkg(local_path, local, "ecmp");
                            open_pkg(remote_path, remote, "ecmp");

                            // Compare the versions
                            if(strcmp(local->version, remote->version) != 0)
                            {
                                    msg(INFO, "package %s is at version %s, available version is %s", local->name, local->version, remote->version);
                                    msg(INFO, "upgrading %s from %s to %s", local->name, local->version, remote->version);
                                    uninstall(local->name);

                                    f_install_package_source(remote_path, 0, local_repo);
                                    new_version_installed = 1;
                            }
    
                            free(local);
                            free(remote);
                        }
                    }
                    // Free each file path string
                    free(searched_files_array[j]);
                }
                // Free each file path string
                free(files_array[i]);
            }
            // Free the array of file paths
            free(searched_files_array);
        }
        // Free the array of file paths
        free(files_array);
    } 
    else
    {
        // If no files found, print a message
        printf("No files found.\n");
    }
    
    if(new_version_installed == 0)
    {
        msg(WARNING, "all packages are up to date");
    }
    
    return 0;
}
