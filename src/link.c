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

        // This is a variable that stores the targets  
        char** target;
        char* buffer = calloc(count * (PATH_MAX + 64) + 1, sizeof(char));

        // This loops over the links and finds their target
        for(int i = 0; i < count; i++)
        {
            // Allcoaion for variable that store the command to find the target
            char* read_link_cmd = calloc(PATH_MAX + 64, sizeof(char));
            sprintf(read_link_cmd, "readlink %s/%s", build_loc, links[i]);
            char* find_cmd = calloc(PATH_MAX + 64, sizeof(char));

            // Check if the link is relative
            if(strchr(exec(read_link_cmd), '/') != NULL)
            {
                // If not, use the output of readlink
                // This probably won't work
                snprintf(find_cmd, PATH_MAX + 64, "%s", exec(read_link_cmd));

                // Executes the search command to find the target
                // Target[strcspn(buffer, "\n")] = 0; removes the new line
                // Not needed for now
                strcat(buffer, find_cmd);
            }
            else
            {   
                // If true, use the location of the link + output of readlink

                // Will extract the location of the link
                char *lastSlash = strrchr(links[i], '/');

                // Calculates the length of the substring
                size_t length = lastSlash - links[i] + 1;

                // Creates a substring up to the last '/'
                char substring[length];
                strncpy(substring, links[i], length);
                substring[length - 1] = '\0'; // Null-terminate the substring
    
                sprintf(find_cmd, "%s/%s", substring, exec(read_link_cmd));

                // Executes the search command to find the target
                // Target[strcspn(buffer, "\n")] = 0; removes the new line
                // Not needed for now
                strcat(buffer, find_cmd);
            }
            
            free(read_link_cmd);
            free(find_cmd);
        }

        // Log the retrieved target locations for debugging
        dbg(3, "Got targets: '%s'", buffer);

        // Split the 'buffer' string into an array of target locations using '\n' as a delimiter
        unsigned int target_count = splita(buffer, '\n', &target);

        // Log the count of retrieved target locations for debugging
        dbg(2, "Got %d targets", target_count);

        if(target_count != 0)
        {

            // This is a variable that stores the link command  
            char** link_cmd;
            char* link_cmd_buffer = calloc(count * (PATH_MAX + 64) + 1, sizeof(char));

            if(target_count != count)
            {
                msg(FATAL, "Some links created by the program appear to be non-functional");
            }

            // This loops over the links and creates a linking command for each
            for(int i = 0; i < count; i++)
            {
                // A command to link the link to the target
                // This assumes every link had a target, if not, this will break
                // Too bad
                char* link_cmd_tmp = calloc(PATH_MAX + 64, sizeof(char));
                sprintf(link_cmd_tmp, "ln -sfv %s %s", target[i], links[i]);
                strcat(link_cmd_tmp, "\n");
                strcat(link_cmd_buffer, link_cmd_tmp);

                free(link_cmd_tmp);
            }

            dbg(4, "Linking command is: %s", link_cmd_buffer);

            unsigned int link_cmd_count = splita(link_cmd_buffer, '\n', &link_cmd);
            
            // This removes the duplicate target entries
            // Since multiple links can lead to the same target
            // I know we have a hashmap, but like, yea
            for (int i = 0; i < target_count - 1; i++) 
            {
                for (int j = i + 1; j < target_count;) 
                {
                    if (strcmp(target[i], target[j]) == 0) 
                    {
                        // Move the duplicate string to the end
                        char* temp = target[j];
                        for (int k = j; k < target_count - 1; k++)
                        {
                            target[k] = target[k + 1];
                        }
                        target[target_count - 1] = temp;
                        target_count--;
                    }
                    else
                    {
                        j++;
                    }
                }
            }

            // This loops over the targets and moves them to their new location 
            for(int i = 0; i < target_count; i++)
            {
                // will create the file 'start' and the file 'end'
                // In order to cerate the link in the new dir
                char* start = calloc(PATH_MAX + 64, sizeof(char));
                char* end = calloc(PATH_MAX + 64, sizeof(char));
                sprintf(start, "%s/%s", build_loc, target[i]);
                sprintf(end, "%s/%s", dest_loc, target[i]);
            

                if (!(access(end, F_OK) == 0))
                {
                    if (end == NULL) {
                        msg(FATAL, "Location is NULL");
                    }

                    // Move the files from 'start' to 'end' location
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

                free(start);
                free(end);
            }

            // This loops over the link commands and executes them
            for(int i = 0; i < link_cmd_count; i++)
            {
                // Executes the link command
                dbg(2, "executing %s now", link_cmd[i]);
                system(link_cmd[i]);
            }
            
            free(buffer);
        }
        free(target);
        free(links);
    }
}
