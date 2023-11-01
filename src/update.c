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
int update()
{
    msg(INFO, "listing installed packages from %s", getenv("INSTALLED_DB"));

    //shame that print_all_data uses msg, this could have been so clean
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    // Prepare the SQL query
    const char *sql = "SELECT Name, Version, Type FROM Packages";
    rc = sqlite3_prepare_v2(INSTALLED_DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }

    // Execute the SQL query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("\x1b[31;1;1m %s \x1b[0m %s - %s \n", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}

int upgrade()
{
    msg(INFO, "listing installed packages from %s", getenv("INSTALLED_DB"));

    //shame that print_all_data uses msg, this could have been so clean
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    // Prepare the SQL query
    const char *sql = "SELECT Name, Version, Type FROM Packages";
    rc = sqlite3_prepare_v2(INSTALLED_DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }

    // Execute the SQL query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("\x1b[31;1;1m %s \x1b[0m %s - %s \n", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}
