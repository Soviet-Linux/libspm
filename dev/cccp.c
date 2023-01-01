#include <stdio.h>
#include <stdlib.h>

#include "../include/libspm.h"
#include "../include/utils.h"
#include "../include/hashtable.h"



int _install_source_(unsigned int* index);
int _remove_(unsigned int* index);
int _install_repo_(unsigned int* index);
int _create_binary_from_file(unsigned int* i);

int _set_debug_level_(unsigned int* i);
int _set_debug_unit(unsigned int* i);
int _set_verbose_(unsigned int* i);
int _set_overwrite_(unsigned int* i);


void* args[][2] = {
    {"package",_install_source_},
    {"install",_install_repo_},
    {"remove",_remove_},
    {"debug",_set_debug_level_},
    {"unit",_set_debug_unit},
    {"verbose", _set_verbose_},
    {"overwrite", _set_overwrite_},
    {"create", _create_binary_from_file}
};

char** ARGV;

int main(int argc, char** argv) {

    if (argc < 2) {
        msg(ERROR, "No command specified");
        return 1;
    }

    hashtable* hm = hm_init(args, sizeof(args)/sizeof(args[0]));

    ARGV = argv;

    init();

    int (*func)(int*);
    for (int i = 1; i < argc; i++) {
        dbg(1, "argv[%d] = %s", i, argv[i]);

        //hm_visualize(hm);
        // function pointer

        // get function pointer
        func = hm_get(hm, argv[i]);
        if (func == NULL) {
            msg(ERROR, "Invalid argument %s", argv[i]);
            return 1;
        }

        // call function
        func(&i);
    }


}

int _install_source_(unsigned int* i) {
    exit(install_package_source(ARGV[++(*i)],0));
}
int _remove_(unsigned int* i) {
    exit(uninstall(ARGV[++(*i)]));
}
int _install_repo_(unsigned int* i) {
    struct package* pkg = calloc(1, sizeof(struct package));
    pkg->name = ARGV[++(*i)];

    char* format = get(pkg, pkg->name);

    if (format == NULL) {
        msg(ERROR, "Failed to download package %s", pkg->name);
        return 1;
    }

    f_install_package_source(pkg->name, 0, format);

    remove(pkg->name);
    return 0;
}


int _set_debug_level_(unsigned int* i) {
    DEBUG = atoi(ARGV[++(*i)]);
    return 0;
}
int _set_debug_unit(unsigned int* i) {
    DEBUG_UNIT = ARGV[++(*i)];
    return 0;
}
int _set_verbose_(unsigned int* i) {
    QUIET = false;
    return 0;
}
int _set_overwrite_(unsigned int* i) {
    OVERWRITE = true;
    return 0;
}

int _create_binary_from_file(unsigned int* i) {
    char* file = ARGV[++(*i)];
    char* binary = ARGV[++(*i)];

    create_binary_from_source(file,binary);

    return 0;
}