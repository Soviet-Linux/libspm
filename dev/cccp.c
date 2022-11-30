#include <stdio.h>

#include "libspm.h"
#include "utils.h"
#include "hashtable.h"

int DEBUG=3;

int _install_(char** args);
int _remove_(char** args);

void* args[][2] = {
    {"install",_install_},
    {"remove",_remove_}
};

int main(int argc, char** argv) {

    if (argc < 3) {
        msg(ERROR, "No command specified");
        return 1;
    }
    dbg(1, "Command: %s", argv[1]);
    hashtable* hm = hm_init(args, sizeof(args)/sizeof(args[0]));
    //hm_visualize(hm);
    // function pointer
    int (*func)(char**);
    // get function pointer
    func = hm_get(hm, argv[1]);
    if (func == NULL) {
        msg(ERROR, "Invalid command");
        return 1;
    }
    init();
    // call function
    return func(&argv[2]);

}

int _install_(char** args) {
    return installSpmFile(args[0],0);
}
int _remove_(char** args) {
    return uninstall(args[0]);
}
