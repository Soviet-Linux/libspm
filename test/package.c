// Tests disabled for now
#if 0
// this file will be used by the our to check a package's validity.
// it replaces in this usage the test.c file, now only used for libspm testing.

#include "test.h"


#define STATIC

char WORKING_DIR[2048];

int main(int argc, char const *argv[])
{


    // check for arguments
    if (argc  < 2)
    {
        printf("No arguments provided\n");
        return 1;
    }

    // set debug level
    setenv("SOVIET_DEBUG","3",1);

    char* spm_path = strdup(argv[1]);

    // check if file is a valid package
    msg(INFO,"Checking package validity...");
    test_ecmp(spm_path);
    msg(INFO,"Package is valid");
    msg(INFO,"Installing package...");
    test_pm(spm_path);
    msg(INFO,"Package installed successfully");

    return 0;
}

#endif