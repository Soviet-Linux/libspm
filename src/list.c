#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sqlite3.h"      // SQLite database library

// Include necessary headers
#include "libspm.h"
#include "cutils.h"

//should probably add there to the header when we are done

//will print the content of INSTALLED_DB
int list_installed()
{
    dbg(2, "listing installed packages from %s", getenv("INSTALLED_DB"));

    //shame that print_all_data uses msg, this could have been so clean
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    // Prepare the SQL query
    const char *sql = "SELECT Name, Version, Type FROM Packages";
    rc = sqlite3_prepare_v2(INSTALLED_DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", zErrMsg); // compiler doesn't complain about this but it might be bad
        sqlite3_free(zErrMsg);
        return 1;
    }

    // Execute the SQL query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("\x1b[31;1;1m %s \x1b[0m %s - %s \n", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(INSTALLED_DB));
        return -1;
    }
    
    dbg(2, "%d packages installed", count_installed());
    return 0;
}

//count installed
int count_installed()
{
    int count = 0;

    sqlite3_stmt *stmt;
    int rc;
    
    // Prepare the SQL query
    const char *sql = "SELECT COUNT(*) FROM Packages";
    rc = sqlite3_prepare_v2(INSTALLED_DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(INSTALLED_DB));
        return 1;
    }
    // Execute the SQL query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        count = (int)sqlite3_column_int(stmt, 0);
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        msg(ERROR, "Error executing statement: %s\n", sqlite3_errmsg(INSTALLED_DB));
        return -1;
    }


    return count;
}

int search(char* in)
{
    msg(INFO, "searching for %s", in);
    
    sqlite3_stmt *stmt;
    int rc;
    int _found = 0;

    // Prepare the SQL query
    const char *sql = "SELECT Name, Section FROM Packages";
    rc = sqlite3_prepare_v2(ALL_DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(ALL_DB));
        return 1;
    }
    
    // Execute the SQL query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        struct package* remote = calloc(1, sizeof(struct package));
        remote->name = (char*)sqlite3_column_text(stmt, 0);
        
        if(strstr(remote->name, in) != 0)
        {
             printf("found \x1b[31;1;1m %s \x1b[0m in %s \n", remote->name, (char*)sqlite3_column_text(stmt, 1));
             _found++;
        }
        free(remote);
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(ALL_DB));
        return -1;
    }

    msg(WARNING, "found %d packages that match %s", _found, in);

    return 0;
}
