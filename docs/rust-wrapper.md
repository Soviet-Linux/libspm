# Explaining 
The libspm is the core of the Soviet Linux package manager. It is a library that is used to install packages and manage them.
Its ment to be called by any other program that needs to install packages.

The main frontend is the CCCP , the soviet llinux command line package manager.
The CCCP is written in rust , and is used by the user to install and manager package.
The CCCP will have to parse user input and call the libspm.


# Important functions 

The CCCP will have to call the following functions:

### Install functions 

 - `int installSpmFile(char* spm_path,int as_dep)`This function will install a package from a source spm file.
 - `int installSpmBinary(char* archivePath,int as_dep)` This function will install a package from a binary spm file.
 - `int check (const char* name)` check package integrity

### Remove function
 - `int uninstall(char* name)` This function will uninstall a package.




### Binary function
 - `int createBinary(char* spm_path,char* bin_path)` This function will create a binary spm file from a source spm file.

### Misc
 - `int get(struct package *i_pkg,char* out_path)` This function will dwnload a package from the repositories. (The _OUR_)
 - `void sync ()` This function will sync packqge database with the repositories. (The _OUR_)
 - `int msg(enum level msgLevel, const char* message,...)` This function will print a message to the user using the spm debugging style.
 - `dbg(int level, const char* message,...)` This function will print a message to the user using the spm debugging style. (_note this is a macro, so it will probably be impossible to port , if you want to try anyway loo for the `int f_dbg__(int level,int line,const char* function,const char* file,char* message,...)` function in the libspm source code_)
 - `void init()`  This function will initialize the libspm.

### We need that now
 - `int open_pkg(char* path, struct package* pkg,char* format)` This function will open a package from a file.
 - `int create_pkg(char* path,struct package* pkg,char* format)` This function will create a package struct.

### Less important functions
 - `int get_all_data(char * DB_PATH)`  This function will get the data (type name version)from all packages in the package database.   


