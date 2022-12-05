#pragma once


#include "globals.h"

#define LIBSPM_VERSION 0.502

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
};
struct package
{
    // Basic infos
    char* name;
    char* type; // for the type at first i used an enum but im lazy and its stupid;
    char* version;
    char* license;
    char* url;

    char** dependencies;
    int dependenciesCount;

    char** makedependencies;
    int makedependenciesCount;

    char** optionaldependencies;
    int optionaldependenciesCount;

    char ** locations;
    int locationsCount;

    // cmds
    struct cmd info;

};

// package info


// shared function to be called by external programs

// This prints the version , its bad 
// TODO: Rework this
float version();

//# Package manipulation 

// install packages
//main exec function
/*-->*/int f_install_package_source(const char* spm_path,int as_dep,const char* format); // install a package file with a provided format
/*-->*/ __attribute__((unused)) int install_package_source(const char* spm_path,int as_dep); // install a package file with the format provided in the file
//
/*-->*/int f_install_package_binary(const char* spm_path,int as_dep,const char* format); // install a package file with a provided format
/*-->*/ __attribute__((unused)) int install_package_binary(char* archivePath,int as_dep);
// Remove packages
int uninstall(char* name);
// Check packages
int check (const char* name);


//get a package 
/* the return value is a package format*/
char* get(struct package *i_pkg,char* out_path);

// update the system
int update();
// clean the work dirs
int clean();

// init the system
void init();
// free everything and quit
void quit(int status);

int readConfig(char* configFilePath);


//open a pkg file (can be spm or ecmp)
int open_pkg(const char* path, struct package* pkg,const char* format);
int create_pkg(const char* path,struct package* pkg,const char* format);

int runFormatLib (const char* format,const char* fn,const char* pkg_path,struct package* pkg);






