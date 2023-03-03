
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


 // class stuff
#include "libspm.h"
#include "globals.h"
#include "cutils.h"
#include "data.h"



char* get(struct package *i_pkg,const char* out_path)
{
    // I commented this part because the soviet system im working on right now doesnt support curl 
    // I will add it later whan the rest of the stuff is ready 

    // check if ALL_FILE exists
    if (i_pkg->name == NULL)
    {
        msg(ERROR,"Package name not specified!");
        return NULL;
    }

    dbg(3,"Loading package %s from ALL_DB\n",i_pkg->name);
    

    char* pkg_format = calloc(64,sizeof(char));
    char* pkg_section = calloc(64,sizeof(char));
    

    retrieve_data_repo(ALL_DB,i_pkg,&pkg_format,&pkg_section);
    dbg(3,"Got %s %s %s %s from DB",pkg_format,pkg_section,i_pkg->version,i_pkg->type);
    if (!pkg_format || !pkg_section || !i_pkg->version || !i_pkg->type) {
        msg(FATAL,"Failed to retrieve data from ALL_DB");
        return NULL;
    }
    dbg(3,"format is '%s'\n",pkg_format);
    dbg(3,"section is '%s'\n",pkg_section);

    // we need to add a way to check if the repo that we are using is on the same version as the one in the database
    // if not , we need to update the database*

    dbg(1,"Downloading %s %s %s",i_pkg->name,i_pkg->version,i_pkg->type);

    // loop through REPOS
    char url[64+strlen(i_pkg->type)+strlen(i_pkg->name)+strlen(pkg_format)];
    sprintf(url,"%s/%s/%s.%s",pkg_section,i_pkg->type,i_pkg->name,pkg_format);

    if (downloadRepo(url, out_path) != 0)
    {
        msg(ERROR,"Failed to download %s",url);
        return NULL;
    } 

    free(pkg_section);
    return pkg_format;
}

void sync ()
{
    downloadRepo("all.db", getenv("ALL_DB_PATH"));
}

