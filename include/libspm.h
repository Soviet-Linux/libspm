#pragma once

#include "cutils.h"
#include "globals.h"

#define LIBSPM_VERSION 1.000

#define SOURCE "src"
#define BINARY "bin"

struct cmd
{
    // Commands
    char* test;
    char* prepare;
    char* install;
    char* special;
};

struct package
{
    // Basic infos
    char* name;
    char* type; // for the type at first i used an enum but im lazy and its stupid;
    char* version;
    char* license;
    char* url;
    char* environment;

    // Internal
    char* path;
    char* format;
    //

    char* description;

    char** files;
    int filesCount;

    char** dependencies;
    int dependenciesCount;

    char** optional;
    int optionalCount;
    
    char ** config;
    int configCount;

    char ** locations;
    int locationsCount;
    
    // cmds
    struct cmd info;

};

struct packages
{
    int count;
    int size;
    struct package* buffer;
};

/*check.c*/
// Function to check if a package is installed and untouched
int check(struct package* pkg);
// Function to check the existence of package locations
int check_locations(char** locations, int locationsCount);

/*clean.c*/
// Function to clean working directories
int clean();
// Function to clean unneeded files after install
void clean_install();

/*config.c*/
// Read a config file
int readConfig(const char* configFilePath, int overwrite);

/*globals.c*/
/*see globals.h*/

/*hashtable.c*/
/*see hashtable.h*/

/*init.c*/
// Function to initialize the Soviet Package Manager
void init();

/*install.c*/
// Function to install a package from source
int install_package_source(struct package* pkg);
// Function to write the package configuration file
void write_package_configuration_file(struct package* pkg);
// Function to read the package configuration file
void read_package_configuration_file(struct package* pkg);

/*make.c*/
// Function to build and install a package
int make(struct package* pkg);

/*move.c*/
// Function to move binaries to the correct locations
void move_binaries(char** locations, long loc_size);

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
int open_pkg(const char* path, struct package* pkg);
// Create a package at the given path using the specified format and package structure
int create_pkg(char* in_path, struct package* pkg);
// Function to free memory allocated for a package structure
int free_pkg(struct package* pkg);
// Function to check if a given package is already in the file tree
int is_installed_pkg(char* path, struct package* pkg);
// Create the database that stores all packages in a directory
int create_pkg_db(char* db_path, struct packages* pkgs);
// Get all packages from a directory
struct packages get_pkgs(char* path);

/*repo.c*/
// Get currently present repos
int get_repos(char** list);
// Function to synchronize the local repository with a remote repository
int repo_sync();
// Add a new repository from a git repo
int add_repo(char* name, char* url);

/*uninstall.c*/
// Function to uninstall packages
int uninstall(char* name);

/*update.c*/
// Function to update a package
int update();

/*util.c*/
// Recursively remove a directory and its contents
int rmrf(char *path);
// remove a file or link or directory
int rmany(char* path);
// Quit the program with the given status code and display an error message if status is not 0
void quit(int status);
// Function to recursively retrieve all files in a directory and its subdirectories
char **get_all_files(const char* root, char *path, int *num_files);
// Load a format plugin, execute a specific function, and close the plugin
int runFormatLib(const char* format, const char* fn, const char* pkg_path, struct package* pkg);
// A function to retrieve the version number of the libspm library.
float version();
// This will parse a string for environment variables
// It makes an assumption that a variable is: $A-Z_0-9
int parse_env(char** in);
// Download a file from url into FILE 
int download(char* url, FILE* fp);
// Copy a file
int cp(char* from, char* to);