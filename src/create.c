#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libspm.h"
#include "cutils.h"


int create_archive(const char* DIR,const char* out_path);

int create_binary_from_source(const char* spm_path,const char* bin_path) {
    // i din't created a macro since i might add logic

    return f_create_binary_from_source(spm_path,bin_path,NULL,getenv("SOVIET_DEFAULT_FORMAT"));
}

int f_create_binary_from_source(const char* src_path,const char* bin_path,const char* in_format,const char* out_format)
{
    struct package pkg;

    open_pkg(src_path, &pkg,NULL);

    PACKAGE_QUEUE[QUEUE_COUNT] = pkg.name; // add this shit to the PKG_QUEUE ARRAY
    QUEUE_COUNT++;
    dbg(1,"Added %s at QUEUE[%d] ",pkg.name,QUEUE_COUNT);


    /* 
        here we have some problems:
            The legacy package dir was in MAKE_DIR/$NAME-$VERSION
        Should we keep it or not ? 
        I choose for compatibility reasons to keep it.
        If soeone wants to chnage this you can vote here :
         - keep it : 1
         - chnage it  : 0
    */
    const char* MAKE_DIR = getenv("SOVIET_MAKE_DIR");
    const char* BUILD_DIR = getenv("SOVIET_BUILD_DIR");

    char legacy_dir[MAX_PATH];
    sprintf(legacy_dir,"%s/%s-%s",MAKE_DIR,pkg.name,pkg.version);
    dbg(1,"legacy dir : %s",legacy_dir);

    // making the package
    if (make(legacy_dir,&pkg) !=0) {
        msg(FATAL,"Make failed");
    }
    dbg(1,"Make done - %s",pkg.name);


    // getting locations
    dbg(1,"Getting locations - %s",pkg.name);
    pkg.locationsCount = get_locations(&pkg.locations,BUILD_DIR);



    char file_path[MAX_PATH];
    sprintf(file_path, "%s/%s.%s",BUILD_DIR,pkg.name,getenv("SOVIET_DEFAULT_FORMAT"));
    create_pkg(file_path,&pkg,NULL);


    // compressing stuff to package archive
    dbg(1,"Compressing binaries - %s",pkg.name);
    create_archive(BUILD_DIR,bin_path);

    clean();

    return 0;
}

int create_archive(const char* DIR,const char* out_path)
{
    char* archive_cmd = calloc(256,sizeof(char));
    sprintf(archive_cmd,"( cd %s && tar -czf %s . )",DIR,out_path);
    dbg(1,"archive_cmd: %s",archive_cmd);
    int EXIT = system(archive_cmd);
    free(archive_cmd);
    return EXIT;
}