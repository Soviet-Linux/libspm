#include "stdio.h"

#include "libspm.h"
#include "globals.h"

#include <utils.h>
#include <stdlib.h>
#include <string.h>




/* 
This readconfig fucntion takes a config file path as agument and initianlize global variable with it.
The current implementation is stupid and bloated.
TODO: Rework it !
 // USE THE HASHMAP TO STORE THE CONFIG VALUES
*/
int readConfig(char* configFilePath)
{
    dbg(2,"config: %s",configFilePath);
    FILE* file = fopen(configFilePath, "r"); /* should check the result */
    char line[1024];
    char* kvlist[2];
    int count;

    while (fgets(line, sizeof(line), file)) {
        /* note that fgets don't strip the terminating \n, checking its
           presence would allow to handle lines longer that sizeof(line) */
        // removing the '\n' mentioned above
        line[strlen(line)-1] = 0;

        char* key = strtok(line,"=");
        char* value = strtok(NULL,"=");
        if (key == NULL || value == NULL) {
            msg(ERROR,"Invalid config file");
            return 1;
        }

        dbg(3,"Key: %s Value: %s",key,value);

        if (strcmp(key,"ROOT") == 0)
        {
            strcpy(ROOT,value);
        }
        else if (strcmp(key,"MAIN_DIR") == 0)
        {

            strcpy(MAIN_DIR,value);
        }
        else if (strcmp(key,"WORK_DIR") == 0)
        {

            strcpy(WORK_DIR,value);
        }
        else if (strcmp(key,"INSTALLED_DB") == 0)
        {

            strcpy(INSTALLED_DB_PATH,value);
        }
        else if (strcmp(key,"ALL_DB") == 0)
        {
            strcpy(ALL_DB_PATH,value);
        }
        else if (strcmp(key,"CONFIG_FILE") == 0)
        {

            strcpy(CONFIG_FILE,value);
        }
        else if (strcmp(key,"REPOS") == 0)
        {
            dbg(3,"REPOS: %s",value);
            strcpy(REPO_ALLOC,value);
            REPO_COUNT = splitm(REPO_ALLOC,' ',REPOS,MAX_REPOS);  
        }
        else if (strcmp(key,"FORMATS") == 0)
        {
            dbg(3,"FORMATS: %s",value);
            strcpy(FORMAT_ALLOC,value);
            FORMAT_COUNT = splitm(FORMAT_ALLOC,' ',FORMATS,MAX_FORMATS);
        }
        else {
            msg(ERROR,"Unknown key in config file : %s",key);
        }
    }
    /* may check feof here to make a difference between eof and io failure -- network
       timeout for instance */

    fclose(file);

    return 0;

}
/*
smol comment to remember how the config file works :

ROOT=/
MAIN_DIR=/var/cccp
DATA_DIR=/var/cccp/data
SPM_DIR=/var/cccp/spm
...etc...

REPOS= http://localhost:8080/ 
*/