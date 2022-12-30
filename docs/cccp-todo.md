# CCCP - Libspm

### Commands
For the first release, we need to have the following functionality:
 - Install packages (ecmp,...) :
    - From a local file  
            `cccp install --file <package-file>`  
            `cccp install -f <package-file>`
    - From a repository  
            `cccp install <package-name>`
 - Remove packages from the system :    
            `cccp remove <package-name>`
 - List packages :
    - List all packages  
            `cccp list all`
    - List installed packages  
            `cccp list installed`
    - List packages from a given repository  
            `cccp list -r <repository-name>`
 - Update packages :
    - Update a package
            `cccp update <package-name>`
    - Update all packages  
            `cccp update all`
 - Search packages :  
    - Seach in all packages   
            `cccp search <package-name>`
    - Search packages from a given repository   
            `cccp search -r <repository-name> <package-name>`

### Additional args
    - `--help` : Show help for the command
    - `--version` : Show version of the command
    - `--verbose` : Show verbose output
    - `--debug <int>` : Show debug output with the specified level
    - `--overwite` : Overwrite existing files
    - `--force` : Force the command to run

### Variables
    - `ROOT` : system root (where to install packages)
    - `REPOS` : List of repositories to use
    - `MAIN_DIR` : cccp home directory
    - `WORK_DIR` : Where to build and store packages
    - `MAKE_DIR` : Where we make packages
    - `BUILD_DIR` : Where pakages binaries are installed 
    - `CCCP_LOG_FILE` : Log file to use
    - `CCCP_LOG_FORMAT` : Log format to use
    - `CCCP_LOG_DATE_FORMAT` : Log date format to use

        
    


