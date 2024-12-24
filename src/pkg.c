#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "cutils.h"
#include "libspm.h"

// Allocate an array of packages
struct packages* create_pkgs(int reserve)
{
	struct packages* pkgs = calloc(1, sizeof(struct packages));
	pkgs->size = 1 + reserve;
	pkgs->count = 0;
	pkgs->buffer = (struct package*)calloc(1 + reserve, sizeof(struct package));
	return pkgs;
}

// Merge 2 package arrays
void merge_pkgs(struct packages* destination, struct packages* source)
{
    for(int i = 0; source->count != 0; i++)
    {
        struct package* pkg = pop_pkg(source);
        push_pkg(destination, pkg);
    }
    free(source->buffer);
    free(source);
}

// Free a package array
void free_pkgs(struct packages* pkgs)
{
    for(int i = 0; pkgs->count != 0; i++)
    {
        struct package* pkg = pop_pkg(pkgs);
        free_pkg(pkg);
    }

    free(pkgs->buffer);    
    free(pkgs);    
}

// Push a package into the array
void push_pkg(struct packages* pkgs, struct package* pkg)
{
	if (pkgs->count == pkgs->size)
	{
		pkgs->buffer = realloc(pkgs->buffer, sizeof(struct package)*(pkgs->count + pkgs->size));
		pkgs->size = pkgs->count + pkgs->size;
	}
    (pkgs->buffer[pkgs->count]) = *pkg;
	pkgs->count++;
}

// Pop the last added package from the array
struct package* pop_pkg(struct packages* pkgs)
{
	pkgs->count--;
	return &(pkgs->buffer[pkgs->count]);
}

// Open a package from the given path and populate the package structure
int open_pkg(const char* path, struct package* pkg)
{
    char full_path[MAX_PATH];
    sprintf(full_path, "%s/%s", path, pkg->path);
    dbg(3, "path: %s", full_path);
    // Check if the file exists
    if (access(path, F_OK) != 0) {
        msg(ERROR, "File %s does not exist\n", full_path);
        return 1;
    }

    dbg(2, "Getting format from file extension");
    char* format = strrchr(full_path, '.') + 1;
    dbg(1, "Format: %s", format);

    char** FORMATS;
    int FORMAT_COUNT = splita(getenv("SOVIET_FORMATS"), ' ', &FORMATS);

    if (format != NULL) {
        // This is experimental
        for (int i = 0; i < FORMAT_COUNT; i++)
        {
            dbg(2, "format: %s = %s", format, FORMATS[i]);
            if (strcmp(format, FORMATS[i]) == 0) 
            {
                dbg(2, "Opening package with %s format", FORMATS[i]);
                runFormatLib(FORMATS[i], "open", full_path, pkg);
                free(FORMATS);
                return 0;
            }
        }
    } else {
        msg(ERROR, "File %s is not a valid package file", full_path);
        return 1;
    }
    msg(ERROR, "File %s is not a valid package file, or the format plugin isn't loaded", full_path);
    return 1;
}

// Create a package at the given path using the specified package structure
int create_pkg(char* in_path, struct package* pkg) 
{
    char path[MAX_PATH];
    sprintf(path, "%s/%s", in_path, pkg->path);

    // This might seem stupid
    // and it is
    if(isdir(path) != 0)
    { 
        pmkdir(path);
        rmany(path);
    }

    dbg(2, "Creating package %s", path);

    dbg(2, "Getting format from file extension");
    char* format = strrchr(path, '.') + 1;
    dbg(1, "Format: %s", format);

    char** FORMATS;
    int FORMAT_COUNT = splita(getenv("SOVIET_FORMATS"),' ',&FORMATS);
    
    if (format != NULL)
    {
        // this is experimental
        for (int i = 0; i < FORMAT_COUNT; i++)
        {
            if (strcmp(format, FORMATS[i]) == 0)
            {
                dbg(2, "Opening package with %s format", FORMATS[i]);
                runFormatLib(FORMATS[i], "create", path, pkg);
                free(FORMATS);
                return 0;
            }
        }
    }
    msg(ERROR,"File %s is not a valid package file, or the format plugin isn't loaded", path);
    free(FORMATS);
    return 1;
}

// Function to free memory allocated for a package structure
int free_pkg(struct package* pkg) 
{
    if (pkg->name != NULL) free(pkg->name);
    if (pkg->version != NULL) free(pkg->version);
    if (pkg->license != NULL) free(pkg->license);
    if (pkg->type != NULL) free(pkg->type);
    if (pkg->url != NULL) free(pkg->url);
    if (pkg->description != NULL) free(pkg->description);
    if (pkg->environment != NULL) free(pkg->environment);
    if (pkg->path != NULL) free(pkg->path);

    if (pkg->info.install != NULL) free(pkg->info.install);
    if (pkg->info.special != NULL) free(pkg->info.special);
    if (pkg->info.prepare != NULL) free(pkg->info.prepare);
    if (pkg->info.test != NULL) free(pkg->info.test);

    if (pkg->locations) 
    {
        for(int i = 0; i < pkg->locationsCount; i++)
        {
            free(pkg->locations[i]);
        }
        free(pkg->locations);
    }
    if (pkg->dependencies) 
    {
        for(int i = 0; i < pkg->dependenciesCount; i++)
        {
            free(pkg->dependencies[i]);
        }
        free(pkg->dependencies);
    }
    if (pkg->optional) 
    {
        for(int i = 0; i < pkg->optionalCount; i++)
        {
            free(pkg->optional[i]);
        }
        free(pkg->optional);
    }
    if (pkg->files) 
    {
        for(int i = 0; i < pkg->filesCount; i++)
        {
            free(pkg->files[i]);
        }
        free(pkg->files);
    }
    if (pkg->config) 
    {
        for(int i = 0; i < pkg->configCount; i++)
        {
            free(pkg->config[i]);
        }
        free(pkg->config);
    }
    return 0;
}

// Function to search for a package in the database
struct packages* search_pkgs(char* db_path, char* term)
{
    sqlite3* db;
    int result = sqlite3_open(db_path, &db);
    if(result) {msg(ERROR, "SQL error when opening SOVIET_DB");}
    struct packages* pkgs = create_pkgs(256 /*absolutely random number*/);

    // Prepare the SQL query
    // yes, this is a copy-paste from what we had a year ago
    sqlite3_stmt* stmt;
    result = sqlite3_prepare_v2(db, "SELECT Name, Path FROM Packages", -1, &stmt, NULL);
    if (result != SQLITE_OK) { msg(ERROR, "SQL error when preparing to search"); }

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        // TODO:
        // This very is stupid
        const unsigned char* name = sqlite3_column_text(stmt, 0);
        const unsigned char* path = sqlite3_column_text(stmt, 1);
        if(strstr((const char*)name, term) != 0)
        {
            dbg(2, "FOUND: %s at %s", name, path);
            struct package pkg = {0};
            pkg.name = strdup((const char*)name);
            pkg.path = strdup((const char*)path);
            push_pkg(pkgs, &pkg);
        }
    }

    result = sqlite3_finalize(stmt);                        
    if (result != SQLITE_OK)     {   msg(ERROR, "SQL error when finalizing statement"); }

    sqlite3_close(db);
    return pkgs;
}

// Create the database that stores all packages in a directory
int create_pkg_db(char* db_path, struct packages* pkgs)
{
    rmany(db_path);
    sqlite3* db;
    int result = sqlite3_open(db_path, &db);
    if(result) {msg(ERROR, "SQL error when creating SOVIET_DB"); return result;}

    result = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Packages (Name TEXT, Path TEXT)", NULL, NULL, NULL);
    if (result != SQLITE_OK) {msg(ERROR, "SQL error when creating pkg_table"); return result;}

    for(int i = 0; i < pkgs->count; i++)
    {
        sqlite3_stmt* stmt;
        result = sqlite3_prepare_v2(db, "INSERT INTO Packages VALUES (?,?)", -1, &stmt, NULL);  if (result != SQLITE_OK)     {   msg(ERROR, "SQL error when preparing to insert package");  return result;  }
        result = sqlite3_bind_text(stmt, 1, pkgs->buffer[i].name, -1, NULL);                    if (result != SQLITE_OK)     {   msg(ERROR, "SQL error when binding package name");         return result;  }
        result = sqlite3_bind_text(stmt, 2, pkgs->buffer[i].path, -1, NULL);                    if (result != SQLITE_OK)     {   msg(ERROR, "SQL error when binding package path");         return result;  }
        result = sqlite3_step(stmt);                                                            if (result != SQLITE_DONE)   {   msg(ERROR, "SQL error when inserting package");            return result;  }
        result = sqlite3_reset(stmt);                                                           if (result != SQLITE_OK)     {   msg(ERROR, "SQL error when resetting statement");          return result;  }
        result = sqlite3_finalize(stmt);                                                        if (result != SQLITE_OK)     {   msg(ERROR, "SQL error when finalizing statement");         return result;  }

    } 

    sqlite3_close(db);
    return 0;
}

// Get all packages from a directory
struct packages* get_pkgs(char* path)
{
    // NOTE:
    // it only checks the default format
    // not sure if that's a good idea
    int num_files;
    char **files_array = get_all_files(path, path, &num_files);
    if (files_array != NULL) 
    {
        struct packages* pkgs = create_pkgs(num_files);
        // Print each file path
        for (int j = 0; j < num_files; j++) 
        {
            if(strstr(files_array[j], "/.") == NULL)
            {          
                if(strstr(files_array[j], getenv("SOVIET_DEFAULT_FORMAT")) != NULL)
                {
                    if(strstr(files_array[j], getenv("SOVIET_DEFAULT_FORMAT"))[strlen(getenv("SOVIET_DEFAULT_FORMAT"))] == '\0')
                    {
                        dbg(1, "file: %s", files_array[j]);

                        char path[MAX_PATH];
                        sprintf(path, "%s", files_array[j]);
                        char name[MAX_PATH];
                        sprintf(name, "%s", files_array[j]);

                        while(strrchr(name, '/') != NULL)
                        {
                            memmove(name, strrchr(name, '/') + 1, strlen(strrchr(name, '/') + 1) + 1);
                        }

                        name[strlen(name) - (strlen(getenv("SOVIET_DEFAULT_FORMAT")) + 1) /*+1 for the '.'*/] = '\0';

                        dbg(1, "path is %s, name is %s", path, name);

                        struct package pkg = {0};
                        pkg.name = strdup(name);
                        pkg.path = strdup(path);

                        push_pkg(pkgs, &pkg);
                    }
                    // Garbage file
                }
                // Not "SOVIET_DEFAULT_FORMAT" file
            }
            // "hidden" file
            free(files_array[j]);
        }
        // Free the array of file paths
        free(files_array);
        if(pkgs->count < 1)
        {
            msg(ERROR, "Path %s does not contain any packages", path);  
            struct packages* pkgs = create_pkgs(0);
            return pkgs;
        }
        return pkgs;
    } 
    else 
    {
        msg(ERROR, "Path %s empty", path);  
        struct packages* pkgs = create_pkgs(0);
        return pkgs;
    }
}

// Function to dump a database into an array of packages
struct packages* dump_db(char* db_path)
{
    sqlite3* db;
    int result = sqlite3_open(db_path, &db);
    if(result) {msg(ERROR, "SQL error when opening SOVIET_DB");}
    struct packages* pkgs = create_pkgs(256 /*absolutely random number*/);

    // Prepare the SQL query
    // yes, this is a copy-paste from what we had a year ago
    sqlite3_stmt* stmt;
    result = sqlite3_prepare_v2(db, "SELECT Name, Path FROM Packages", -1, &stmt, NULL);
    if (result != SQLITE_OK) { msg(ERROR, "SQL error when preparing to search"); }

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW) 
    {
        const unsigned char* name = sqlite3_column_text(stmt, 0);
        const unsigned char* path = sqlite3_column_text(stmt, 1);

        struct package pkg = {0};

        pkg.name = strdup((const char*)name);
        pkg.path = strdup((const char*)path);
        push_pkg(pkgs, &pkg);
    }

    result = sqlite3_finalize(stmt);                        
    if (result != SQLITE_OK)     {   msg(ERROR, "SQL error when finalizing statement"); }

    sqlite3_close(db);
    return pkgs;
}

// Function returns an array of packages that need updating
struct packages* update_pkg(struct package* pkg)
{
    msg(INFO, "fetching updates");
    int new_version_found = 0;
    
    if(access(getenv("SOVIET_INSTALLED_DB"), F_OK) != 0)
    {
        msg(ERROR, "no installed DB found");
        return -1;
    }

    struct packages* installed_packages = dump_db(getenv("SOVIET_INSTALLED_DB"));

    if(new_version_found == 0) { msg(INFO, "all packages are up to date"); }
    return 0;
}