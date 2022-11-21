
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "spm/libspm.h"
#include "spm/globals.h"
#include "spm/utils.h"


void quit(int status)
{
    if (status != 0)
    {
        msg(ERROR, "Exiting with status %d", status);
        exit(status);
    }
    return;
    
}

