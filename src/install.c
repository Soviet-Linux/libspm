#define _GNU_SOURCE

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// class stuff
#include "libspm.h"
#include "cutils.h"
#include "data.h"



/*
In this function we are installing source or binary packages.
Source packages are archive files containing the source code of the package ,the post install script and an .spm file for the install commands.
Binary packages are archive files containing the compiled binary files of the package , the post install script and an .spm file for the install commands.
*/


// parsing data and installing package archive (with sources)

int install_package_source(const char* spm_path,int as_dep) {
    return f_install_package_source(spm_path,as_dep,NULL);
}

int f_install_package_source(const char* spm_path,int as_dep,const char* format)
{
    if (spm_path == NULL) {
        msg(ERROR,"spm_path is NULL");
        return -1;
    }
    char* make_dir = getenv("SOVIET_MAKE_DIR");
    char* build_dir = getenv("SOVIET_BUILD_DIR");
    if (make_dir == NULL || build_dir == NULL) {
        msg(ERROR,"SOVIET_MAKE_DIR or SOVIET_BUILD_DIR is not set");
        return -1;
    }

    struct package pkg = {0};

    if (open_pkg(spm_path, &pkg,format) != 0) {
        msg(ERROR,"Failed to open package");
        return -1;
    }
    
    msg(INFO, "Installing %s", pkg.name);

    PACKAGE_QUEUE[QUEUE_COUNT] = pkg.name; // add this shit to the PKG_QUEUE ARRAY
    QUEUE_COUNT++;

    dbg(1,"Added %s to the queue",pkg.name);

    /*
        the following code is pretty bad.
        I'll let i like that for now wince its needed
        but I'll try to improve it later

        TODO: Fix this shit
    */


    if (pkg.dependencies != NULL && pkg.dependenciesCount > 0 && strlen(pkg.dependencies[0]) > 0)

    {
        dbg(1,"Checking dependencies...");
        check_dependencies(pkg.dependencies,pkg.dependenciesCount);
    }
    // checking makedeps
    if (pkg.makedependencies != NULL && pkg.makedependenciesCount > 0 && strlen(pkg.makedependencies[0]) > 0)
    {
        dbg(3,"Checking makedeps : %s",pkg.makedependencies);
        check_dependencies(pkg.makedependencies,pkg.makedependenciesCount);

    }


    /* 
        here we have some problems:
            The legacy package dir was in MAKE_DIR/$NAME-$VERSION
        Should we keep it or not ? 
        I choose for compatibility reasons to keep it.
        If soeone wants to chnage this you can vote here :
         - keep it : 1
         - chnage it  : 0
    */

    char legacy_dir[MAX_PATH];
    sprintf(legacy_dir,"%s/%s-%s",getenv("SOVIET_MAKE_DIR"),pkg.name,pkg.version);


    // making the package
    dbg(1,"Making %s",pkg.name);
    if (make(legacy_dir,&pkg) != 0) {
        msg(ERROR,"Failed to make %s",pkg.name);
        return -1;
    }
    dbg(1,"Making %s done",pkg.name);


    // getting locations
    dbg(1,"Getting locations for %s",pkg.name);
    pkg.locationsCount = get_locations(&pkg.locations,getenv("SOVIET_BUILD_DIR"));
    if (pkg.locationsCount <= 0) {
        msg(ERROR,"Failed to get locations for %s",pkg.name);
        return -1;
    }
    dbg(1,"Got %d locations for %s",pkg.locationsCount,pkg.name);

    // check if package is already installed
    if (is_installed(pkg.name))
    {
    msg(WARNING,"Package %s is already installed, reinstalling",pkg.name);
        uninstall(pkg.name);
    }
    else {
        dbg(3,"Package %s is not installed",pkg.name);
    }
    
    // moving binaries
    dbg(1,"Moving binaries for %s",pkg.name);
    move_binaries(pkg.locations,pkg.locationsCount);

    //  executing post install scripts
    // check if pkg.info.special is not empty or NULL
    if (pkg.info.special != NULL && strlen(pkg.info.special) > 0)
    {
        dbg(1,"Executing post install script for %s",pkg.name);
        exec_special(pkg.info.special,getenv("SOVIET_BUILD_DIR"));
    }
   
    // remove the deprecated unsafe format function call
    // format the path using sprintf
    char file_path[MAX_PATH];
    sprintf(file_path,"%s/%s.%s",getenv("SOVIET_SPM_DIR"),pkg.name,getenv("SOVIET_DEFAULT_FORMAT"));
    create_pkg(file_path,&pkg,NULL);

    store_data_installed(INSTALLED_DB,&pkg ,as_dep);

    // now we need to clean everything 
    clean();
    // remove package from the queue
    QUEUE_COUNT--;
    PACKAGE_QUEUE[QUEUE_COUNT] = NULL;

    free_pkg(&pkg);
    return 0;
}
/*
Utilities declaration for binary install
*/
// untar a binary package to another dir
int uncompress_binary(const char* bin_path,const char* dest_dir);

__attribute__((unused)) int install_package_binary(const char* archivePath,int as_dep)
{
    struct package pkg;

    char* default_format = getenv("SOVIET_DEFAULT_FORMAT");
    char* build_dir = getenv("SOVIET_BUILD_DIR");
    char* spm_dir = getenv("SOVIET_SPM_DIR");

    if (default_format == NULL || build_dir == NULL || spm_dir == NULL)
    {
        msg(ERROR,"Environment variables not set");
        return -1;
    }

    pkg.name = calloc(sizeof(archivePath),sizeof(char));
    
    if (get_bin_name(archivePath,pkg.name) != 0)
    {
        msg(ERROR,"Could not get name from archive path");
        return -1;
    }

    //uncompressing binary and checking output
    if (uncompress_binary(archivePath,build_dir) != 0) return -1;

    // format the path using sprintf
    char spm_path[MAX_PATH];
    sprintf(spm_path,"%s/%s.%s",build_dir,pkg.name,default_format);
    if (access(spm_path,F_OK) != 0)
    {
        msg(ERROR,"%s not found",spm_path);
        return -1;
    }


    open_pkg(spm_path,&pkg,NULL);

    PACKAGE_QUEUE[QUEUE_COUNT] = pkg.name; // add this shit to the PKG_QUEUE ARRAY
    QUEUE_COUNT++;
    dbg(1,"Added %s to QUEUE[%d]",pkg.name,QUEUE_COUNT-1);

    check_dependencies(pkg.dependencies,pkg.dependenciesCount);

        // moving binaries
    dbg(1,"Moving binaries for %s",pkg.name);
    move_binaries(pkg.locations,pkg.locationsCount);

    

    //  executing post install scripts
    exec_special(pkg.info.special,build_dir);


    char file_path[MAX_PATH];
    sprintf(file_path,"%s/%s.%s",spm_dir,pkg.name,default_format);
    create_pkg(file_path,&pkg,NULL);

    store_data_installed(INSTALLED_DB,&pkg ,as_dep);

    // now we need to clean everything
    clean();
    // remove package from the queue
    QUEUE_COUNT--;
    PACKAGE_QUEUE[QUEUE_COUNT] = NULL;

    //free_pkg(&pkg);
    return 0;

    // now we need to clean everything 
    clean();



    //free_pkg(&pkg);

    return 0;
}

/*
Private utilities
*/

int uncompress_binary(const char* bin_path,const char* dest_dir)
{
    // format the path using sprintf
    char untar_cmd[strlen(bin_path)+strlen(dest_dir)+64];
    sprintf(untar_cmd,"tar -xvf %s -C %s",bin_path,dest_dir);

    return system(untar_cmd);
}


/*
Shared utilities
*/

bool is_installed(const char* name)
{
    char path[1024];
    char** FORMATS;
    int FORMAT_COUNT = splita(strdup(getenv("SOVIET_FORMATS")),' ',&FORMATS);

    // loop through all formats
    for (int i = 0; i < FORMAT_COUNT; i++)
    {
        sprintf(path,"%s/%s.%s",getenv("SOVIET_SPM_DIR"),name,FORMATS[i]);
        if (access(path,F_OK) == 0)
        {
            free(*FORMATS);
            free(FORMATS);
            return true;
        }
    }
    free(*FORMATS);
    free(FORMATS);
    return false;
}

int get_bin_name(const char* bin_path,char* name)
{
    const char* file_name = strrchr(bin_path, '/');
    if (file_name == NULL)
        file_name = bin_path;
    else
        file_name++;
    for (int i = 0; i < (int)strlen(file_name); i++) {
        if (file_name[i] == '.') {
            sprintf(name,"%.*s",i,file_name);
            return 0;
        }
    }
    return -1;
}

int free_pkg(struct package *pkg) {
    if (pkg->name != NULL) free(pkg->name);
    if (pkg->version != NULL) free(pkg->version);
    if (pkg->license != NULL) free(pkg->license);
    if (pkg->type != NULL) free(pkg->type);
    if (pkg->url != NULL) free(pkg->url);

    if (pkg->info.make != NULL) free(pkg->info.make);
    if (pkg->info.special != NULL) free(pkg->info.special);
    if (pkg->info.download != NULL) free(pkg->info.download);
    if (pkg->info.install != NULL) free(pkg->info.install);
    if (pkg->info.prepare != NULL) free(pkg->info.install);
    if (pkg->info.test != NULL) free(pkg->info.test);

    if (*pkg->locations != NULL) free(*pkg->locations);
    if (pkg->locations != NULL) free(pkg->locations);

    if (*pkg->dependencies != NULL) free(*pkg->dependencies);
    if (pkg->dependencies != NULL) free(pkg->dependencies);

    if (*pkg->makedependencies != NULL) free(*pkg->makedependencies);
    if (pkg->makedependencies != NULL) free(pkg->makedependencies);

    if (*pkg->makedependencies != NULL) free(*pkg->makedependencies);
    if (pkg->makedependencies != NULL) free(pkg->makedependencies);

    if (*pkg->optionaldependencies != NULL) free(*pkg->optionaldependencies);
    if (pkg->optionaldependencies != NULL) free(pkg->optionaldependencies);

    return 0;
}















