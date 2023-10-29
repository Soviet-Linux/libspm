#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include <dlfcn.h>
#include <stdlib.h>

#include "cutils.h"
#include "libspm.h"



int open_pkg(const char* path, struct package* pkg,const char* format)
{
    dbg(2,"Setting everything to NULL"); 
    //set all variables t NULL
    memset(pkg,0,sizeof(struct package));


    // print make dependencies count
    dbg(3,"make dependencies count : %d",pkg->makedependenciesCount);

    // check if file exists
    if (access(path,F_OK) != 0)
    {
        msg(ERROR,"File %s does not exist\n",path);
        return 1;
    }
    //check file extension


    /* This illustrates strrchr */

    if (format == NULL)
    {
        dbg(2,"Getting format from file extension");
        format = strrchr( path, '.' ) + 1;
        dbg(1,"Format : %s\n",format);
    }  
    
    char** FORMATS;
    int FORMAT_COUNT = splita(getenv("SOVIET_FORMATS"),' ',&FORMATS); 

    if (format != NULL)
    {
        // this is experimental
        for (int i = 0; i < FORMAT_COUNT; i++)
        {
            printf("format : %s = %s\n",format,FORMATS[i]);
            if (strcmp(format,FORMATS[i]) == 0)
            {
                dbg(2,"Opening package with %s format",FORMATS[i]);
                runFormatLib(FORMATS[i],"open",path,pkg);
                return 0;
            }
        }
    }
    else
    {
        msg(ERROR,"File %s is not a valid package file",path);
        return 1;
    }
    msg(ERROR,"File %s is not a valid package file, or the format plugin isn't loaded",path);
    return 1;

}

int create_pkg(const char* path,struct package* pkg,const char* format)
{
    msg(INFO,"Creating package %s",path);

    char** FORMATS;
    int FORMAT_COUNT = splita(strdup(getenv("SOVIET_FORMATS")),' ',&FORMATS);

    // get file extension
    if (format == NULL)
    {
        format = strrchr( path, '.' ) + 1;
    } 
    /* This illustrates strrchr */
    if (format != NULL)
    {
        // this is experimental
        for (int i = 0; i < FORMAT_COUNT; i++)
        {
            if (strcmp(format,FORMATS[i]) == 0)
            {
                dbg(2,"Opening package with %s format",FORMATS[i]);
                runFormatLib(FORMATS[i],"create",path,pkg);
                //free(*FORMATS);
                free(FORMATS);
                return 0;
            }
        }
    }
    msg(ERROR,"File %s is not a valid package file, or the format plugin isn't loaded",path);
    //free(*FORMATS);
    free(FORMATS);
    return -1;
}

int runFormatLib (const char* format,const char* fn,const char* pkg_path,struct package* pkg)
{
    char lib_path[MAX_PATH];
    sprintf(lib_path,"%s/%s.so",getenv("SOVIET_PLUGIN_DIR"),format);
    dbg(2,"Loading %s",lib_path);

    if (access(lib_path,F_OK) != 0)
    {
        msg(ERROR,"File %s does not exist",lib_path);
        return 1;
    }
    
    // load fn from so lib
    void* handle = dlopen(lib_path,RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr,"%s\n",dlerror());
        return 1;
    }
    int (*func)(const char*,struct package*) = dlsym(handle,fn);
    char* error = dlerror();
    if (error != NULL)
    {
        fprintf(stderr,"%s\n",error);
        return 1;
    }
    if (func(pkg_path,pkg) != 0)
    {
        return -1;
    }

    dlclose(handle);
    return 0;

}
