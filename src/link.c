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

    // Allocate space for links
    char** links;

    // Split the 'res' string into an array of file locations using '\n' as a delimiter
    unsigned int count = splita(res, '\n', &links);

    // Log the count of retrieved locations for debugging
    dbg(2, "Got %d locations", count);

    char* link_cmd = calloc(count * (PATH_MAX + 64) + 1, sizeof(char));
    for(int i = 0; i < count; i++)
    {
        char* read_link_cmd = calloc(PATH_MAX + 64, sizeof(char));
        char* find_cmd = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(read_link_cmd, "readlink %s/%s", build_loc, links[i]);

        sprintf(find_cmd, "( cd %s && find . -name %s ", build_loc, exec(read_link_cmd));
        find_cmd[strcspn(find_cmd, "\n")] = 0;

        char* tmp_1 = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(tmp_1, " | cut -c2- ) ");
        strcat(find_cmd, tmp_1);

        char* target = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(target, exec(find_cmd));
        target[strcspn(target, "\n")] = 0;

        char* tmp = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(link_cmd, "ln -sfv %s %s", target, links[i]);

        char* start = calloc(PATH_MAX + 64, sizeof(char));
        char* end = calloc(PATH_MAX + 64, sizeof(char));
        sprintf(start, "%s/%s", build_loc, target);
        sprintf(end, "%s/%s", dest_loc, target);

        if (!(access(end, F_OK) == 0))
        {
            if (end == NULL) {
                msg(FATAL, "Location is NULL");
            }

            switch (mvsp(start, end))
            {
                case -1:
                    msg(FATAL, "Moving failed, could not create dir");
                    break;
                case -2:
                    msg(FATAL, "Moving failed, destination not a dir");
                    break;
                case 0:
                    msg(WARNING, "Moved %s to %s", start, end);
                    break;
            }
        }
        else 
        {
            msg(WARNING, "%s is already here, use --overwrite?", end);
            if (OVERWRITE) {
                // Rename the file in the build directory to the destination location
                rename(start, end);
            } else {
                msg(FATAL, "Terminating the program");
            }
        }

        dbg(2, "executing %s now", link_cmd);

        int result = system(link_cmd);

        free(read_link_cmd);
        free(find_cmd);
        free(tmp_1);
        free(target);
        free(start);
        free(end);
    }
    free(links);
}