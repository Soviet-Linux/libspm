#include "string.h"
#include "unistd.h"
#include "sqlite3.h"
#include "stdio.h"

#include "libspm.h"
#include "utils.h"

int connect_db(sqlite3 **db,char* DB_PATH) {
    int rc = sqlite3_open(DB_PATH, db);
    if (rc) {
        msg(ERROR, "Can't open database: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return 1;
    }
    return 0;
}

int create_table_installed(sqlite3 *db) {
    char *err_msg = NULL;
    int rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Packages (Name TEXT, Version TEXT, Type TEXT, AsDep INT)", 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }


    return 0;
}


int store_data_installed (sqlite3 *db, struct package *pkg,int as_dep) {
    sqlite3_stmt *stmt;  // Statement handle.
    int rc;  // Return code.

    // Prepare the INSERT statement.
    rc = sqlite3_prepare_v2(db,
                            "INSERT INTO Packages (Name, Version, Type, AsDep) VALUES (?, ?, ?, ?)",
                            -1,
                            &stmt,
                            NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "Error preparing INSERT statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Bind the values to the INSERT statement.
    sqlite3_bind_text(stmt, 1, pkg->name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, pkg->version, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, pkg->type, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, as_dep);

    // Execute the INSERT statement.
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        msg(ERROR, "Error executing INSERT statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    dbg(1,"Inserted %s %s %s -- exit : %d ",pkg->name,pkg->version,pkg->type,rc);
    // Finalize the statement.
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        msg(ERROR, "Error finalizing INSERT statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

int retrieve_data_installed(sqlite3 *db, struct package *pkg,int* as_dep) {
    sqlite3_stmt *stmt;
    int rc;

    dbg(1,"Retrieving %s data from DB",pkg->name);

    // Prepare the SQL query
    const char *sql = "SELECT Version, Type, AsDep FROM Packages WHERE Name = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if( rc != SQLITE_OK ){
        msg(ERROR, "SQL error: %s -- %d \n", sqlite3_errmsg(db),rc);
        return -1;
    }
    dbg(2,"SQL query: %s - exit : %d",sql,rc);

    // Bind the value for the parameter in the SQL query
    sqlite3_bind_text(stmt, 1, pkg->name, -1, SQLITE_STATIC);

    while( (rc = sqlite3_step(stmt)) == SQLITE_ROW ){
        pkg->version = strdup((char*)sqlite3_column_text(stmt, 0));
        pkg->type  = strdup((char*)sqlite3_column_text(stmt, 1));
        if (as_dep != NULL) *as_dep = sqlite3_column_int(stmt, 2);
    }

    // Check if the SQL query was successful
    if( rc != SQLITE_DONE ){
        msg(ERROR, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

int remove_data_installed(sqlite3* db,char* name)
{
    sqlite3_stmt *stmt;
    int rc;

    // Prepare the SQL query
    const char *sql = "DELETE FROM Packages WHERE Name = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if( rc != SQLITE_OK ){
        msg(ERROR, "SQL error: %s -- %d", sqlite3_errmsg(db),rc);
        return 1;
    }

    // Bind the value for the parameter in the SQL query
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

    // Execute the INSERT statement.
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        msg(ERROR, "Error executing INSERT statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Finalize the statement.
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        msg(ERROR, "Error finalizing INSERT statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}


int retrieve_data_repo(sqlite3 *db, struct package *pkg,char** format,char** section) {
    sqlite3_stmt *stmt;
    int rc; 

    dbg(1,"Retrieving %s data from repo DB",pkg->name);

    // Prepare the SQL query
    const char *sql = "SELECT Version, Type, Format, Section FROM Packages WHERE Name = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if( rc != SQLITE_OK ){
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(db));
        return 1;
    }

    // Bind the value for the parameter in the SQL query
    sqlite3_bind_text(stmt, 1, pkg->name, -1, SQLITE_STATIC);

    while( (rc = sqlite3_step(stmt)) == SQLITE_ROW ){
        pkg->version = strdup((const char*)sqlite3_column_text(stmt, 0));
        pkg->type  = strdup((const char*)sqlite3_column_text(stmt, 1));
        if (format != NULL) (*format) = strdup((const char*)sqlite3_column_text(stmt, 2));
        if (section != NULL) (*section) = strdup((const char*)sqlite3_column_text(stmt, 3));
    }

    // Check if the SQL query was successful
    if( rc != SQLITE_DONE ){
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

int print_all_data(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    // Prepare the SQL query
    const char *sql = "SELECT Name, Version, Type FROM Packages";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    dbg(1,"rc = %d",rc);
    if( rc != SQLITE_OK ){
        msg(ERROR, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }


    // Execute the SQL query
    while( (rc = sqlite3_step(stmt)) == SQLITE_ROW ){
        msg(INFO,"%s ==> %s - %s", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
    }

    // Check if the SQL query was successful
    if( rc != SQLITE_DONE ){
        fprintf(stderr, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}






