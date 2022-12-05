#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// class stuff
#include "libspm.h"
#include "utils.h"

// This function will check if all dependencies of a package are installed
int check_dependencies (char ** dependencies,int dependenciesCount) 
{
    dbg(1,"Checking dependencies...");


    for (int i = 0; i < dependenciesCount; i++)
    {
        dbg(3,"Checking if %s is installed",dependencies[i]);
        if (!is_installed(dependencies[i]))
        {
            dbg(3, "Dependency %s is not installed",dependencies[i]);
            // TODO: we need to install the dependencie
            msg(INFO,"Installing %s",dependencies[i]);

            // check if the dependencie is in the queue
            int in_queue = 0;
            for (int j = 0; j < QUEUE_COUNT; j++)
            {
                if (strcmp(PACKAGE_QUEUE[j],dependencies[i]) == 0)
                {
                    in_queue = 1;
                    break;
                }
            }
            if (in_queue)
            {
                dbg(1,"Package %s is already in the queue",dependencies[i]);
                continue;
            }

            char dep_path[PATH_MAX];
            snprintf(dep_path,PATH_MAX,"/tmp/%s-dep.tmp",dependencies[i]);
            struct package dep_pkg = {0};
            dep_pkg.name = dependencies[i];
            // get the dependency
            char* dep_format = get(&dep_pkg,dep_path);
            if (dep_format == NULL)
            {
                msg(ERROR,"Failed to get dependency %s",dependencies[i]);
                return -1;
            }
            // install the dependency



            /*
                TODO: Find a clever way to implement automatic dependency installing*
                In The meantime i'll implement no dep-checking.
            */

            return 0;
        }
        else 
        {
            dbg(3, "Dependency %s is installed",dependencies[i]);
        }
    }

   
    return 0;

}

