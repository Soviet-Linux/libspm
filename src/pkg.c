#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "cutils.h"
#include "libspm.h"

// Allocate an array of packages
struct packages create_pkgs(int reserve)
{
	packages pkgs;
	pkgs.size = 1 + reserve;
	pkgs.count = 0;
	pkgs.buffer = (struct package*)calloc(1 + reserve, sizeof(struct package));
	return pkgs;
}

// Merge 2 package arrays
void merge_pkgs(struct packages* destination, struct packages* source)
{
    for(int i = 0; i < source->count; i++)
    {
        pkg = pop_pkg(source);
        push_pkg(destination, pkg);
        free_pkg(pkg);
    }
}

// Push a package into the array
void push_pkg(struct packages* pkgs, struct package* pkg)
{
	if (pkgs->count == pkgs->size)
	{
		pkgs->buffer = realloc(pkgs->buffer, sizeof(struct package)*(pkgs->count + pkgs->size));
		pkgs->size = pkgs->count + pkgs->size;
	}
    memcpy(pkgs->buffer[pkgs->count], pkg, sizeof(struct package));
	pkgs->count++;
}

// Pop the last added package from the array
struct package* pop_pkg(struct packages* pkgs)
{
	pkgs->count--;
	struct package* pkg = calloc(1, sizeof(struct package));
    memcpy(pkg, pkgs->buffer[pkgs->count], sizeof(struct package));
	free_pkg(&(pkgs->buffer[pkgs->count]));
	return pkg;
}


// Open a package from the given path and populate the package structure
int open_pkg(const char* path, struct package* pkg, const char* format)
{
    dbg(2, "Setting everything to NULL");
    // Set all variables to NULL
    memset(pkg, 0, sizeof(struct package));

    // Print make dependencies count
    dbg(3, "make dependencies count: %d", pkg->optionalCount);
    dbg(3, "path: %s", path);

    // Check if the file exists
    if (access(path, F_OK) != 0) {
        msg(ERROR, "File %s does not exist\n", path);
        return 1;
    }

    // Check file extension
    if (format == NULL) {
        dbg(2, "Getting format from file extension");
        format = strrchr(path, '.') + 1;
        dbg(1, "Format: %s\n", format);
    }

    char** FORMATS;
    int FORMAT_COUNT = splita(getenv("SOVIET_FORMATS"), ' ', &FORMATS);

    if (format != NULL) {
        // This is experimental
        for (int i = 0; i < FORMAT_COUNT; i++) {
            dbg(2, "format: %s = %s\n", format, FORMATS[i]);
            if (strcmp(format, FORMATS[i]) == 0) {
                dbg(2, "Opening package with %s format", FORMATS[i]);
                runFormatLib(FORMATS[i], "open", path, pkg);
                return 0;
            }
        }
    } else {
        msg(ERROR, "File %s is not a valid package file", path);
        return 1;
    }
    msg(ERROR, "File %s is not a valid package file, or the format plugin isn't loaded", path);
    return 1;
}

// Create a package at the given path using the specified format and package structure
int create_pkg(struct package* pkg, const char* format) 
{
    char repo_path[MAX_PATH];
    sprintf(repo_path, "%s/%s", getenv("SOVIET_SPM_DIR"), pkg->repo);

    if(isdir(repo_path) != 0)
    { 
        pmkdir(repo_path);
    }
    
    char path[MAX_PATH]; 
    sprintf(path, "%s/%s/%s.%s", getenv("SOVIET_SPM_DIR"), pkg->repo, pkg->name, getenv("SOVIET_DEFAULT_FORMAT"));

    msg(INFO, "Creating package %s", path);

    char** FORMATS;
    int FORMAT_COUNT = splita(strdup(getenv("SOVIET_FORMATS")),' ',&FORMATS);

    // get file extension
    if (format == NULL)
    {
        format = strrchr( path, '.' ) + 1;
    } 
    /* This illustrates strrchr */
    if (format != NULL)
    {
        // this is experimental
        for (int i = 0; i < FORMAT_COUNT; i++)
        {
            if (strcmp(format,FORMATS[i]) == 0)
            {
                dbg(2, "Opening package with %s format", FORMATS[i]);
                runFormatLib(FORMATS[i], "create", path, pkg);
                //free(*FORMATS);
                return 0;
            }
        }
    }
    msg(ERROR,"File %s is not a valid package file, or the format plugin isn't loaded",path);
    //free(*FORMATS);
    free(FORMATS);
    return -1;
}

// Function to free memory allocated for a package structure
int free_pkg(struct package* pkg) 
{
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

// Create the database that stores all packages in a directory
int create_pkg_db(char* db_path, struct packages* pkgs)
{
    remove(db_path);
    sqlite3* db;
    sqlite3_stmt* pkg_table;
    int result = sqlite3_open(db_path, &db);
    if(result)
    {
        msg(FATAL, "SQL error when creating SOVIET_DB");
    }

    result = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Packages (Name TEXT, Path TEXT)", NULL, NULL, NULL);
    if (result != SQLITE_OK) 
    {
        msg(FATAL, "SQL error when creating pkg_table");
    }

    char* ins = "INSERT INTO Packages VALUES (?,?)";

    sqlite3_close(&db);
}

// Get all packages from a directory
struct packages get_pkgs(char* path)
{
    int num_files;
    char **files_array = get_all_files(path, path, &num_files);
    if (files_array != NULL) 
    {
        packages pkgs = create_pkgs(num_files);
        // Print each file path
        for (int j = 0; j < num_files; j++) 
        {
            char* path = strdup(files_array[j]);
            char* name = strdup(files_array[j]);

            while(strtok(package_name, "/"))
            {
                char* tmp = name;
                name = strchr(package_name, '\0') + 1;
                if(*name == '\0')
                {
                    name = tmp;
                    break;
                }
            }

            dbg(1, "path is %s, name is %s", path, name);

            // Free each file path string
            free(files_array[i]);
            free(path);
        }
        // Free the array of file paths
        free(files_array);
    } 
    else 
    {
        msg(ERROR, "Repository %s empty", REPOS[i]);  
    }
}

// Get all packages from a directory
struct packages get_pkgs_from_repos()
{
    char** REPOS = calloc(512,sizeof(char*));
    int num_repos = get_repos(REPOS)
    if(num_repos < 2)
    {   
        msg(FATAL, "No local repositories");  
    }

    struct packages pkgs = create_pkgs(0);
    for(int i = 1; i < num_repos; i++)
    {
        char* path = calloc(MAX_PATH, 1);
        sprintf(path, "%s/%s", getenv("SOVIET_REPOS_DIR"), REPOS[i]);
        struct packages result = get_pkgs(path);
        merge_pkgs(&pkgs, &result);
    }

    return pkgs;
}
