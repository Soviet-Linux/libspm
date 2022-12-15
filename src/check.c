#include "unistd.h"
#include "stdio.h"


// class stuff
#include "libspm.h"
#include "utils.h"


//checking if package is installed and untouched
/*
Exit code signification :
0 = Good , package is installed and fine
1 = Package is not installed (Package data file is absent)
2 = Package is corrupted (package data file is here but with no location info)
3 = Package is corrupted (Some locations arent here)
*/
int check(const char* name)
{
    char dataSpmPath[MAX_PATH];
    sprintf(dataSpmPath,"%s/%s/%s",DATA_DIR,SPM_DIR,name);

    // checkinig if package data file exists
    if (access(dataSpmPath,F_OK) != 0){
        return 1;
    }
    struct package pkg;
    open_pkg(dataSpmPath,&pkg,DEFAULT_FORMAT);
    if (pkg.locationsCount == 0) {
        return 2;
    }

    return check_locations(pkg.locations,pkg.locationsCount);
}

int check_locations(char** locations,int locationsCount)
{
    for (int i = 0; i < locationsCount; i++) {
        if (access(locations[i],F_OK) != 0) {
            return 3; // this is according to the check() spec
        }
    }
    return 0;
}

