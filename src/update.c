#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sqlite3.h"      // SQLite database library

#include "data.h"

// Include necessary headers
#include "libspm.h"
#include "cutils.h"

//should probably add there to the header when we are done

//will print the content of INSTALLED_DB
int update()
{
    msg(INFO, "fetching updates");
    
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;
    int new_version_found = 0;

    // Prepare the SQL query
    const char *sql = "SELECT Name, Version FROM Packages";
    rc = sqlite3_prepare_v2(INSTALLED_DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }
    
    // Execute the SQL query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        struct package* local = calloc(1, sizeof(struct package));
        struct package* remote = calloc(1, sizeof(struct package));
        local->name = (char*)sqlite3_column_text(stmt, 0);
        local->version = (char*)sqlite3_column_text(stmt, 1);
        dbg(1, "don't ask why this is here");
        remote->name = local->name;
        retrieve_data_repo(ALL_DB, remote, NULL, NULL);
        if(remote->version == NULL)
        {
           msg(ERROR, "No package %s exists in repo", local->name);
        }
        else
        {
            if(strcmp(local->version, remote->version) != 0)
            {
                 msg(INFO, "package %s is at version %s, available version is %s", local->name, local->version, remote->version);
                 new_version_found = 1;
            }
        }
        free(local);
        free(remote);
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(INSTALLED_DB));
        sqlite3_free(zErrMsg);
        return -1;
    }
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
    
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;
    int new_version_installed = 0;

    // Prepare the SQL query
    const char *sql = "SELECT Name, Version FROM Packages";
    rc = sqlite3_prepare_v2(INSTALLED_DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }

    // Execute the SQL query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        struct package* local = calloc(1, sizeof(struct package));
        struct package* remote = calloc(1, sizeof(struct package));
        local->name = (char*)sqlite3_column_text(stmt, 0);
        local->version = (char*)sqlite3_column_text(stmt, 1);
        dbg(1, "don't ask why this is here");
        remote->name = local->name;
        retrieve_data_repo(ALL_DB, remote, NULL, NULL);
        if(remote->version == NULL)
        {
           msg(ERROR, "No package %s exists in repo", local->name);
        }
        else
        {
            if(strcmp(local->version, remote->version) != 0)
            {
                msg(INFO, "upgrading %s from %s to %s", local->name, local->version, remote->version);
                uninstall(local->name);
                char* format = get(local, local->name);

                if (format == NULL) {
                    msg(ERROR, "Failed to download package %s", local->name);
                return 1;
                }

                f_install_package_source(local->name, 0, format);
                new_version_installed = 1;
            }
        }
        free(local);
        free(remote);
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(INSTALLED_DB));
        return -1;
    }
    if(new_version_installed == 0)
    {
        msg(WARNING, "all packages are up to date");
    }
    
    return 0;
}
