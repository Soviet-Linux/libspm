#include "stdio.h"
#include <stdlib.h>
#include <string.h>

#include "libspm.h"
#include "globals.h"
#include "cutils.h"

#include <stdbool.h>

// This will parse a string for environment variables
// It makes an assumption that a variable is: $A-Z_0-9

int parse_env(char** in)
{
    dbg(2, "Parsing string %s for env variables", *in);
    char* env = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_";
    char* start = strchr(*in, '$');
    char* end = NULL;
    int i, start_i;

    if (start == NULL)
    {
        return 0;
    }

    dbg(2, "Start: %s", start);

    start_i = strlen(*in) - strlen(start);

    dbg(2, "Start i: %d, from '%d' and '%d'", start_i, strlen(*in), strlen(start));
    
    for (i = 1; i < strlen(start); i++)
    {
        end = strchr(env, start[i]);

        if (end == NULL)
        {
            if(i == 0)
            {
                return 0;
            }

            if(start[i] != '\0')
            {
                end = &start[i];
            }

            break;
        }

        if(i + 1 == strlen(start))
        {
            end = " ";
        }
    }

    char* var = strdup(*in + start_i + 1);
    char* dup_in = calloc(start_i + 1, 1);
    if(start_i != 0)
    {
        snprintf(dup_in, start_i + 1, "%s", *in);
    }
    var[--i] = '\0';

    dbg(2, "Var: %s", var);
    dbg(2, "In: %s",  *in);

    char* full_var = getenv(var);

    if(full_var == NULL)
    {
        return 0;
    }

    dbg(2, "Full var: %s", full_var);

    char* full_in = calloc(strlen(*in) + strlen(full_var) + strlen(end) + 1, 1);

    sprintf(full_in, "%s%s%s", dup_in, full_var, end);
    
    free(*in);
    free(dup_in);
    free(var);
    *in = full_in;

    dbg(2, "Result: %s", *in);

    return parse_env(in);
}