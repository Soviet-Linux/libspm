#include "stdio.h"
#include "stdlib.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


//class stuff
#include "libspm.h"
#include "cutils.h"
#include "globals.h"

/*
    All the complexity in this function and really in this entire project if just because we need to track  files installed by a makefile
    For now we juste install in a separate directory (getenv("BUILD_DIR")) and then move it to the correct location (The real filesystem)
    I tried many solutions to do this like installwatch , checkinstall , but none of them worked for me.
    If you have an idea PLEASE , I beg you tell me or write it directly here .
    IDEAS :
    - 
    -
    -
    I prefer simple solution that doesnt include to write a timestamp based install-log thing from scratch
    (I tried , but its not good enough)

*/
int make (char* package_dir,struct package* pkg)
{
    char* build_dir = getenv("SOVIET_BUILD_DIR");
    char* make_dir = getenv("SOVIET_MAKE_DIR");

    char *cmd_params;
    if (QUIET) { cmd_params = "&> /dev/null"; }
    else { cmd_params = ""; }

    setenv("BUILD_ROOT",build_dir,1);
    setenv("NAME",pkg->name,1);
    setenv("VERSION",pkg->version,1);

    char excmd[ 64 + strlen(pkg->url)];
    sprintf(excmd,"echo -n %s",pkg->url);
    char* exurl = exec(excmd);
    setenv("URL",exurl,1);
    dbg(3,"url is  : %s\n",exurl);
    free(exurl);
    

    if (pkg->info.download != NULL && strlen(pkg->info.download) > 0) {

        char sources_cmd[ 64 + strlen(make_dir) + strlen(pkg->info.download) ];        

        sprintf(sources_cmd,"(cd %s && %s) %s ",make_dir,pkg->info.download,cmd_params);
        dbg(2,"Downloading sources with %s",sources_cmd);
        int res = system(sources_cmd);

        if ( res != 0) {
            msg(ERROR,"Failed to download sources for %s",pkg->name);
            return -1;
        }
    }
    //checking is the command are used and formatting and executing them
    if (pkg->info.prepare != NULL && strlen(pkg->info.prepare) > 0) 
    {
        //formatting the prepare command
        char prepare_cmd[
            64 + strlen(package_dir) + 
            strlen(pkg->info.prepare) + strlen(cmd_params)
            ];
        

        sprintf(prepare_cmd,"( cd %s && %s ) %s",package_dir,pkg->info.prepare,cmd_params);

        //Printing the command to the terminal
        dbg(2,"Executing prepare command : %s",prepare_cmd);
        //executing the command
        // We add the extra command parameters to the command , so that the user can add extra parameters to the command
        if (system(prepare_cmd) != 0) {
            return 1;
        }
        //debug
        dbg(1,"prepare command executed !");

    }
    dbg(3,"Make command is %s",pkg->info.make);
    if (pkg->info.make && strlen(pkg->info.make)) 
    {
        //formatting the prepare command
        char make_cmd[ 64 + strlen(package_dir) + strlen(pkg->info.make) + strlen(cmd_params) ];
        sprintf(make_cmd, "( cd %s && %s ) %s",package_dir,pkg->info.make,cmd_params);

        //Printing the command to the terminal
        dbg(2,"Executing make command : %s",make_cmd);
        //executing the command
        // We add the extra command parameters to the command , so that the user can add extra parameters to the command
        if (system(make_cmd) != 0) {
            return 1;
        }
        //debug
        dbg(1,"make command executed !");


        
    }
    if (pkg->info.test != NULL && TESTING && strlen(pkg->info.test) > 0) 
    {
        //formatting the  command
        char test_cmd[
            64 +  strlen(package_dir) + 
            strlen(pkg->info.test) + strlen(cmd_params) 
            ];
        sprintf(test_cmd,"( cd %s && %s ) %s",package_dir,pkg->info.test,cmd_params);

        //Printing the command to the terminal
        dbg(2,"Executing test command : %s",test_cmd);
        //executing the command
        if (system(test_cmd) != 0) {
            return 1;
        };
        dbg(1,"make command executed !");
    }
    if (pkg->info.install == NULL && strlen(pkg->info.install) == 0) 
    {
        msg(ERROR,"No install command !");
        return -3;
    }

    //formatting the prepare command
    char install_cmd[ 64 + strlen(package_dir) + strlen(pkg->info.install) + strlen(cmd_params)];
    sprintf(install_cmd, "( cd %s && %s ) %s",package_dir,pkg->info.install,cmd_params);

    //Printing the command to the terminal
    dbg(2,"Executing install command : %s",install_cmd);
    //executing the command
    if (system(install_cmd) != 0) 
    {
        msg(FATAL,"Failed to install %s",pkg->name);
        return -2;
    }
    //debug
    dbg(1,"install command executed !");

    return 0;

}
int exec_special(const char* cmd,const char* package_dir)
{
    dbg(2,"Executing special command : %s",cmd);
    char* special_cmd = calloc(
        64 +strlen(package_dir) + strlen(cmd), 
        sizeof(char));

    if (system(special_cmd) != 0) {
        free(special_cmd);
        return 1;
    }
    free(special_cmd);
    dbg(1,"special command executed !");
    return 0;
}