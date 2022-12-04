#include "unistd.h"
#include "stdio.h"


// class stuff
#include "libspm.h"


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

    for (int i = 0; i < pkg.locationsCount; i++) {
        if (access(pkg.locations[i],F_OK) != 0) {
            return 3;
        }
    }
    return 0;
}
// simpler check that also verifies f the package is being installed 

