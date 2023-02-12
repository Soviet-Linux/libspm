#include "utils.h"
#include <stdlib.h>
#include <sys/stat.h>


int clean()
{
    // cleaning the work dirs
    char* build_dir = getenv("SOVIET_BUILD_DIR");
    char* make_dir = getenv("SOVIET_MAKE_DIR");
    return rmrf(build_dir) + rmrf(make_dir) + mkdir(build_dir,0755) + mkdir(make_dir,0755);

    


}