#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

// Include necessary headers
#include "globals.h"
#include "libspm.h"
#include "cutils.h"


void create_links(char build_loc[PATH_MAX], char dest_loc[PATH_MAX]) 
{
    // Construct a shell command to list links in the specified directory
    char links_location_cmd[PATH_MAX + 64];
    sprintf(links_location_cmd, "( cd %s && find . -type l | cut -c2- ) ", build_loc);

    // Log the constructed command for debugging
    dbg(2, "Getting links locations with %s ", links_location_cmd);

    // Execute the constructed shell command and store the output in 'res'
    char* res = exec(links_location_cmd);

    // Log the retrieved file locations for debugging
    dbg(3, "Got locations: '%s'", res);

    // This is a variable that stores the links
    char** links;

    // Split the 'res' string into an array of file locations using '\n' as a delimiter
    unsigned int count = splita(res, '\n', &links);

    // Log the count of retrieved locations for debugging
    dbg(2, "Got %d locations", count);
    if(count != 0)
    {
        // This loops over the links and finds their target
        for(int i = 0; i < count; i++)
        {
            // Allocoaion for variable that store the command to find the target
            char* read_link_cmd = calloc(PATH_MAX + 64, sizeof(char));
            sprintf(read_link_cmd, "readlink %s/%s", build_loc, links[i]);
            char* target = exec(read_link_cmd);
            char* copy_cmd = calloc(PATH_MAX + 64, sizeof(char));
            char* remove_cmd = calloc(PATH_MAX + 64, sizeof(char));

            target[strcspn(target, "\n")] = 0;

            if(strstr(target, build_loc))
            {
                target = target + strlen(build_loc);
            }

            char* start = calloc(PATH_MAX + 64, sizeof(char));
            char* end = calloc(PATH_MAX + 64, sizeof(char));

            // Check if the link is relative
            if(target[0] == '/')
            {
                sprintf(start, "%s/%s", build_loc, target);
                sprintf(end, "%s/%s", dest_loc, target);
            }
            else
            {
                char* name = strrchr(links[i], '/');
                char* path = calloc(strlen(links[i]) - strlen(name), sizeof(char));
                char* temp_path = calloc(strlen(links[i]), sizeof(char));
                strcat(temp_path, links[i]);
                temp_path[strlen(links[i]) - strlen(name)] = '\0';
                strcat(path, temp_path);
                sprintf(start, "%s/%s/%s", build_loc, path, target);
                sprintf(end, "%s/%s/%s", dest_loc, path, target);
                free(path);
                free(temp_path);
            }

            if ((access(start, F_OK) == 0) && (access(end, F_OK) != 0))
            {
                switch (mvsp(start, end))
                {
                    case -1:
                        msg(FATAL, "Moving %s to %s failed, could not create dir", start, end);
                        break;
                    case -2:
                        msg(FATAL, "Moving %s to %s failed, destination not a dir", start, end);
                        break;
                    case 0:
                        msg(WARNING, "Moved %s to %s", start, end);
                        break;
                }
            }
   
            
            symlink(target, links[i]);
            
            free(read_link_cmd);
            free(copy_cmd);
            free(remove_cmd);
            free(start);
            free(end);

        }
        free(links);
    }
}