#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <git2.h>

// Include custom headers
#include "libspm.h"
#include "globals.h"
#include "cutils.h"

// Get currently present repos
int get_repos(char** list)
{
    dbg(3, "checking for repos");
    DIR *d;
    struct dirent *dir;
    d = opendir(getenv("SOVIET_REPOS_DIR"));
    int count = 0;
    list[count] = calloc(strlen("local") + 1, 1);
    sprintf(list[count], "local");
    count++;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (count > 512)
            {
                printf("Error : too many elements in list , reallocating\n");
                list = realloc(list,(count+512) * sizeof(char*));
            }
            if (dir->d_type != DT_DIR || dir->d_name[0] == '.') continue;

            list[count] = calloc(strlen(dir->d_name) + 1, sizeof(char));
            strcpy(list[count], dir->d_name);
            count++;
        }
    }

    for (int i = 0; i < count - 1; i++)
    {
        if (strcmp(list[i], ".") == 0 || strcmp(list[i], "..") == 0)
        {
            // Move the . string to the end
            char* temp = list[i];
            dbg(3, "Moving: %s", temp);
            for (int k = i; k < count - 1; k++)
            {
                list[k] = list[k + 1];
            }
            list[count - 1] = temp;
            i--;
            count--;
        }
    }

    closedir(d);

    dbg(3, "done checking for repos");
    return count;
}

// Function to synchronize the local repository with a remote repository
int repo_sync() 
{
    char* repo_dir = getenv("SOVIET_REPOS_DIR");
    char* repo_url = getenv("SOVIET_DEFAULT_REPO_URL");
    char* submodule_name = getenv("SOVIET_DEFAULT_REPO");

    // Git repo handle
    git_repository* repo_handle = NULL;
    unsigned int status = 0;

    // bit of debugging
    dbg(3,"SOVIET_REPOS_DIR: %s",repo_dir);
    dbg(3,"SOVIET_DEFAULT_REPO_URL: %s",repo_url);
    dbg(3,"SOVIET_DEFAULT_REPO: %s",submodule_name);

    // Check if the repository directory exists
    if (access(repo_dir, F_OK) != 0) 
    {
        // Create the repository directory if it doesn't exist
        if (pmkdir(repo_dir) != 0) 
        {
            msg(FATAL, "Failed to create directory %s", repo_dir);
        }
    }

    // Check if repo_dir can be accessed
    if (access(repo_dir, X_OK) != 0) 
    {
        msg(FATAL, "%s is not a directory or cannot be accessed.", repo_dir);
    }

    // Check if it's a git repository
    if (git_repository_open(&repo_handle, repo_dir) != 0) 
    {
        // Initialize a new Git repository
        if (git_repository_init(&repo_handle, repo_dir, false) != 0) 
        {
            msg(FATAL, "Failed to initialize git repository in %s.", repo_dir);
        }
    }
    
    // Check if submodule exists
    if(git_submodule_status(&status, repo_handle, submodule_name, GIT_SUBMODULE_IGNORE_ALL) != 0)
    {
        if (add_repo(submodule_name, repo_url) != 0) {msg(ERROR, "Failed to create the default repository");}
    }

    // TODO: add a way to get all submodules and update them
    // But maybe a single system call isn't too bad...
    // Update submodules
    chdir(repo_dir);
    if (system("git submodule update --depth 1 --remote --init --recursive") != 0) 
    {
        printf("Failed to update submodules in %s\n", repo_dir);
        return 3;
    }

    git_repository_free(repo_handle);
    return 0;
}

// Add a new repository from a git repo
int add_repo(char* name, char* url)
{
    const char* repo_dir = getenv("SOVIET_REPOS_DIR");
    git_repository* repo_handle = NULL;

    // Set clone options
    unsigned int* status = NULL;
    git_submodule_update_options opts = {0};
    if(git_submodule_update_options_init(&opts, GIT_SUBMODULE_UPDATE_OPTIONS_VERSION)!= 0)
    {
        msg(FATAL, "Failed to initialize git submodule options");
    }
    git_fetch_options fopts = {0};
    if(git_fetch_options_init(&fopts, GIT_SUBMODULE_UPDATE_OPTIONS_VERSION)!= 0)
    {
        msg(FATAL, "Failed to initialize git fetch options");
    }
    
    opts.fetch_opts = fopts;
    opts.fetch_opts.depth = 1;

    // Check if it's a git repository
    if (git_repository_open(&repo_handle, repo_dir) != 0) 
    {
        msg(ERROR, "%s is not initialized! - run 'cccp --update'", repo_dir);
        return -1;
    }

    // Check if submodule exists
    if(git_submodule_status(status, repo_handle, name, GIT_SUBMODULE_IGNORE_ALL) != 0)
    {
        git_submodule* submodule_handle = NULL;
        git_repository* temp_handle = NULL;

        // Add the submodule
        if(git_submodule_add_setup(&submodule_handle, repo_handle, url, name, 1) != 0)
        {
            const git_error* error = giterr_last();
            msg(ERROR, "Failed to add submodule %s - %s", name, error->message);
            return -1;
        }

        msg(INFO, "Cloning %s into %s/%s...", name, repo_dir, name);

        if(git_submodule_clone(&temp_handle, submodule_handle, &opts) != 0)
        {
            const git_error* error = giterr_last();
            msg(ERROR, "Failed to clone submodule %s - %s", name, error->message);
            return -1;
        }

        if(git_submodule_add_finalize(submodule_handle) != 0)
        {
            const git_error* error = giterr_last();
            msg(ERROR, "Failed to finalize submodule %s - %s", name, error->message);
            return -1;
        }

        git_submodule_free(submodule_handle);
        git_repository_free(temp_handle);
        return 0;
    }

    msg(ERROR, "A repository with the name '%s' already exists.", name);
    return -1;
}