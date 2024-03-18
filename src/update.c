#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Include necessary headers
#include "libspm.h"
#include "cutils.h"

//should probably add there to the header when we are done

//will print the content of INSTALLED_DB
int update()
{
    msg(INFO, "fetching updates");
    
    getenv("SOVIET_SPM_DIR");
    int new_version_found = 0;


    //TODO actually get the versions

    struct package* local = calloc(1, sizeof(struct package));
    struct package* remote = calloc(1, sizeof(struct package));
    if(strcmp(local->version, remote->version) != 0)
    {
            msg(INFO, "package %s is at version %s, available version is %s", local->name, local->version, remote->version);
            new_version_found = 1;
    }
    free(local);
    free(remote);

    if(new_version_found != 0)
    {
        msg(WARNING, "new version found for one or more packages, use --upgrade to upgrade");
    }
    else
    {
        msg(WARNING, "all packages are up to date");
    }
    
    return 0;
}

int upgrade()
{
    msg(INFO, "upgrading");

    msg(INFO, "fetching updates");

    getenv("SOVIET_SPM_DIR");
    int new_version_installed = 0;


    //TODO actually get the versions

    struct package* local = calloc(1, sizeof(struct package));
    struct package* remote = calloc(1, sizeof(struct package));
    if(strcmp(local->version, remote->version) != 0)
    {
        msg(INFO, "upgrading %s from %s to %s", local->name, local->version, remote->version);
        uninstall(local->name);
        char* repo = get(local, local->name);

        if (repo == NULL) {
            msg(ERROR, "Failed to download package %s", local->name);
        return 1;
        }

        f_install_package_source(local->name, 0, repo);
        new_version_installed = 1;
    }
    free(local);
    free(remote);


    if(new_version_installed == 0)
    {
        msg(WARNING, "all packages are up to date");
    }
    
    return 0;
}
