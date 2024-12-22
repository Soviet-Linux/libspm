#pragma once

#include "cutils.h"
#include "globals.h"

#define LIBSPM_VERSION 1.000

#define SOURCE "src"
#define BINARY "bin"

struct cmd
{
    // Commands
    char* make;
    char* test;
    char* prepare;
    char* install;
    char* special;
    char* download;
    char* description;
};

struct package
{
    // Basic infos
    char* name;
    char* type; // for the type at first i used an enum but im lazy and its stupid;
    char* version;
    char* license;
    char* sha256;
    char* url;
    char* environment;

    // Internal
    char* path;
    char* format;
    //

    char** files;
    int filesCount;

    char** dependencies;
    int dependenciesCount;

    char** optional;
    int optionalCount;

    char ** locations;
    int locationsCount;

    char ** inputs;
    int inputsCount;

    char ** exports;
    int exportsCount;


    // cmds
    struct cmd info;

};

struct packages
{
    int count;
    int size;
    struct package* buffer;
};

// List of installed packages
int list_installed();

// num. of installed packages
int count_installed();

// Serach for a package by term
char** search(char *term,  int *num_results);

// Check what packages need updating
int update();

// Upgrade all packages that need updating
int upgrade();

// Create links for the package
void create_links(char build_loc[4096], char dest_loc[4096]);

// Get all repositories currently present
int get_repos(char** list);

// Clone a git repository
int add_repo(char* name, char* url);

// Cleanup unneded files after a package is installed
void clean_install();

// Function to install a package from source with a specific format
int install_package_source(struct package* pkg);

// Function to uninstall packages
int uninstall(char* name);

// Function to check if a package is installed and untouched
int check(const char* name);

// Function to retrieve a package from a data repository
char* get(struct package *i_pkg, const char* repo, const char* out_path);

// Function to move binaries to the correct locations
void move_binaries(char** locations,long loc_size);

// build a package from source
int make (char* package_dir,struct package* pkg);

// execute post install scripts
int exec_special(const char* cmd,const char* package_dir);

// update the system
int update();

// Function to clean working directories
int clean();

// Function to synchronize the local repository with a remote repository
int repo_sync();

// init the system
void init();

// Read the soviet config
int readConfig(const char* configFilePath, int overwrite);

// Function to check the existence of package locations
int check_locations(char** locations,int locationsCount);

// Function to check if a package is already installed
bool is_installed(const char* name);

/*pkg.c*/
// Allocate an array of packages
struct packages create_pkgs(int reserve);
// Merge 2 package arrays
void merge_pkgs(struct packages* destination, struct packages* source);
// Push a package into the array
void push_pkg(struct packages* pkgs, struct package* pkg);
// Pop the last added package from the array
struct package* pop_pkg(struct packages* pkgs);
// Open a package from the given path and populate the package structure
int open_pkg(const char* path, struct package* pkg, const char* format);
// Create a package at the given path using the specified format and package structure
int create_pkg(struct package* pkg);
// Function to free memory allocated for a package structure
int free_pkg(struct package* pkg);
// Create the database that stores all packages in a directory
int create_pkg_db(char* db_path, struct packages* pkgs);
// Get all packages from a directory
struct packages get_pkgs(char* path);

/*util.c*/
// Recursively remove a directory and its contents
int rmrf(char *path);
// remove a file or link or directory
int rmany(char* path);
// Quit the program with the given status code and display an error message if status is not 0
void quit(int status);
// Function to recursively retrieve all files in a directory and its subdirectories
char **get_all_files(const char* root, const char *path, int *num_files);
// Load a format plugin, execute a specific function, and close the plugin
int runFormatLib(const char* format, const char* fn, const char* pkg_path, struct package* pkg);
// A function to retrieve the version number of the libspm library.
float version();
// This will parse a string for environment variables
// It makes an assumption that a variable is: $A-Z_0-9
int parse_env(char** in);
// Download a file from url into FILE 
int download(char* url, FILE* fp);
int cp(char* from, char* to);