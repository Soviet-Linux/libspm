#pragma once

#include "cutils.h"
#include "globals.h"



#define LIBSPM_VERSION 0.503

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

    char** dependencies;
    int dependenciesCount;

    char** optional;
    int optionalCount;

    char ** locations;
    int locationsCount;

    char ** inputs;
    int inputsCount;

    // cmds
    struct cmd info;

};

//test
int list_installed();
int count_installed();
char** search(char *term,  int *num_results);
int update();
int upgrade();
void create_links(char build_loc[4096], char dest_loc[4096]);
int check_optional_dependencies(char ** dependencies,int dependenciesCount);
int get_repos(char** list);
char** getAllFiles(const char* root, const char *path, int *num_files);
//end test

// package info


// shared function to be called by external programs

// This prints the version , its bad 
// TODO: Rework this
float version();

//# Package manipulation 

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
int install_package_source(const char* spm_path,int as_dep);
int f_install_package_source(const char* spm_path, int as_dep,  char* repo);

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
int install_package_binary(const char* archivePath, int as_dep, const char* repo);

// Function to uninstall packages
/*
Accepts:
- char* name: The name of the package to uninstall.

Returns:
- int: An integer indicating the result of the uninstallation.
  - 0: The package was successfully uninstalled.
  - -1: An error occurred during the uninstallation.

Description:
This function is used to uninstall packages. It relies on location data, which contains all the files that were installed by the program. This data is stored in a JSON array inside the package's SPM file in DATA_DIR. The function cycles through all the files in the JSON array and removes them from the system. It also removes the package's entry from the installed packages database.

Note:
- The variable `DEFAULT_FORMAT` is not defined; you may need to replace it with the correct environment variable or value. For example, you can use `getenv("SOVIET_DEFAULT_FORMAT")` or replace it with a string representing the default format.
- The `INSTALLED_DB` variable is assumed to be defined elsewhere in your code.

Please avoid making changes to this code unless there's a critical bug or an important missing feature.
*/
int uninstall(char* name);

// Function to check if a package is installed and untouched
/*
Accepts:
- const char* name: The name of the package to be checked.

Returns:
- int: An integer indicating the result of the check.
  - 0: Good, package is installed and fine.
  - 1: Package is not installed (Package data file is absent).
  - 2: Package is corrupted (package data file is here but with no location info).
  - 3: Package is corrupted (Some locations aren't here).
*/
int check(const char* name);

// Function to create a binary package from source with input and output formats
/*
Accepts:
- const char* src_path: The path to the source package.
- const char* bin_path: The path where the binary package will be created.
- const char* in_format: The input format (optional).
- const char* out_format: The output format.

Returns:
- int: An integer indicating the result of the binary package creation.
*/
int f_create_binary_from_source(const char* src_path,const char* bin_path,const char* in_format,const char* out_format);

// Function to create a binary package from source
/*
Accepts:
- const char* spm_path: The path to the source package.
- const char* bin_path: The path where the binary package will be created.

Returns:
- int: An integer indicating the result of the binary package creation.
*/
int create_binary_from_source(const char* spm_path,const char* bin_path);

// Function to retrieve a package from a data repository
/*
Accepts:
- struct package* i_pkg: A pointer to a package structure with package details.
- const char* out_path: The local path to save the downloaded package.

Returns:
- char*: A pointer to the package format or NULL if there's an error.
*/
char* get(struct package *i_pkg, const char* repo, const char* out_path);

// Function to move binaries to the correct locations
/*
Accepts:
- char** locations: An array of file locations.
- long loc_size: The number of locations in the array.

Description:
This function iterates through the given file locations and moves the binaries to their correct destinations.

Notes:
- It checks if the destination location is empty and moves files from the build directory to the destination.
- If the destination location is not empty, it provides a warning and optionally renames the file in the build directory.

Returns: None
*/
void move_binaries(char** locations,long loc_size);
// build a package from source
int make (char* package_dir,struct package* pkg);
// execute post install scripts
int exec_special(const char* cmd,const char* package_dir);


// update the system
int update();

// Function to clean working directories
/*
Returns:
- int: An integer indicating the result of the cleaning operation.
  - The return value is the sum of the following operations:
    - rmrf(build_dir): Removing the build directory and its contents.
    - rmrf(make_dir): Removing the make directory and its contents.
    - mkdir(build_dir, 0755): Creating the build directory with the specified permissions.
    - mkdir(make_dir, 0755): Creating the make directory with the specified permissions.
*/
int clean();
// Function to synchronize the local repository with a remote repository
int repo_sync();

// init the system
void init();

// Quit the program with the given status code and display an error message if status is not 0
/*
Accepts:
- int status: The exit status code.

Description:
This function exits the program with the specified status code. If the status code is not 0 (indicating an error), it also displays an error message.

Returns:
- void: This function does not return a value.
*/
void quit(int status);

int readConfig(const char* configFilePath);

// Function to check the existence of package locations
/*
Accepts:
- char** locations: An array of strings representing package locations.
- int locationsCount: The number of locations in the array.

Returns:
- int: An integer indicating the result of the check.
  - 0: All locations exist, so the package is installed and fine.
  - 3: Some locations do not exist, indicating package corruption (some locations are missing).
*/
int check_locations(char** locations,int locationsCount);

// Open a package from the given path and populate the package structure
/*
Accepts:
- const char* path: The path to the package file.
- struct package* pkg: A pointer to the package structure to populate.
- const char* format: The format of the package (optional).

Description:
This function opens a package from the specified path, reads the package file's format, and populates the provided package structure with its contents.

Returns:
- int: An integer indicating the result of opening the package.
  - 0: Package opened successfully.
  - 1: File does not exist or is not a valid package file.
  - 1: File is not a valid package file or the format plugin isn't loaded.
*/
int open_pkg(const char* path, struct package* pkg,const char* format);

// Create a package at the given path using the specified format and package structure
/*
Accepts:
- const char* path: The path to the package file to be created.
- struct package* pkg: A pointer to the package structure containing package data.
- const char* format: The format of the package (optional).

Description:
This function creates a package file at the specified path using the provided format and package data from the package structure.

Returns:
- int: An integer indicating the result of creating the package.
  - 0: Package created successfully.
  - -1: File is not a valid package file or the format plugin isn't loaded.
*/
int create_pkg(const char* path,struct package* pkg,const char* format);

// Load a format plugin, execute a specific function, and close the plugin
/*
Accepts:
- const char* format: The format of the package.
- const char* fn: The name of the function to execute in the format plugin.
- const char* pkg_path: The path to the package file.
- struct package* pkg: A pointer to the package structure.

Description:
This function loads a format plugin, executes a specified function within the plugin, and then closes the plugin.

Returns:
- int: An integer indicating the result of running the format plugin.
  - 0: Format plugin executed successfully.
  - 1: Format plugin file does not exist.
  - 1: Error loading or executing the format plugin.
  - -1: Format plugin function returned an error.
*/
int runFormatLib (const char* format,const char* fn,const char* pkg_path,struct package* pkg);

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
int get_bin_name(const char* bin_path,char* name);

// Function to check if a package is already installed
/*
Accepts:
- const char* name: Name of the package to check.

Returns:
- bool: A boolean value indicating whether the package is installed.
  - true: Package is installed.
  - false: Package is not installed.
*/
bool is_installed(const char* name);

// Function to free memory allocated for a package structure
/*
Accepts:
- struct package* pkg: Pointer to a package structure.

Returns:
- int: An integer indicating the result of memory deallocation.
  - 0: Memory freed successfully.
*/
int free_pkg(struct package* pkg);

// Function to download a file from a given URL and save it to a specified path
/*
Accepts:
- const char* url: The URL of the file to download.
- const char* file_path: The local file path to save the downloaded file.

Returns:
- int: An integer indicating the result of the download operation.
  - 0: Download success.
  - -1: Download failure.
*/
char* load_from_repo(const char* in, const char* repo, const char* file_path);

// Function to download a repository from a given URL
/*
Accepts:
- const char* url_path: The path to the resource on the repository.
- const char* file_path: The local file path to save the downloaded resource.

Returns:
- int: An integer indicating the result of the download operation.
  - 0: Download success.
  - 1: Download failure.
*/
int loadFile(const char* path, const char* file_path);

// Function to retrieve file locations within a directory
/*
Accepts:
- char*** locations: Pointer to an array of strings to store file locations.
- const char* loc_dir: Path to the directory to search for files.

Returns:
- long: The number of file locations retrieved.
*/
long get_locations(char*** locations, const char* loc_dir);

// Function to check if all dependencies of a package are installed
/*
Accepts:
- char **dependencies: An array of dependency names.
- int dependenciesCount: The number of dependencies in the array.

Returns:
- int: An integer indicating the result of dependency checking.
  - 0: All dependencies are installed.
  - -1: An error occurred during dependency checking.
*/
int check_dependencies(char ** dependencies,int dependenciesCount);







