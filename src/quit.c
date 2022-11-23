
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "libspm.h"
#include "globals.h"
#include "utils.h"


void quit(int status)
{
    if (status != 0)
    {
        msg(ERROR, "Exiting with status %d", status);
        exit(status);
    }
    return;
    
}

