#include "string.h"        // Standard library for string manipulation
#include "unistd.h"        // Standard library for system calls
#include "sqlite3.h"      // SQLite database library
#include "stdio.h"         // Standard I/O library for file operations

#include "libspm.h"        // Custom library for package management
#include "cutils.h"        // Custom utility library

// Callback function for SQLite queries
int callback(void *NotUsed, int argc, char **argv, char **azColName);

// Function to connect to an SQLite database
/*
Accepts:
- sqlite3 **db: A pointer to an SQLite database handle.
- char *DB_PATH: The path to the SQLite database file.

Returns:
- int: An integer indicating the result of the database connection.
  - 0: Success.
  - 1: Failure, unable to open the database.
*/
int connect_db(sqlite3 **db, char *DB_PATH) {
    int rc = sqlite3_open(DB_PATH, db);
    if (rc) {
        msg(ERROR, "Can't open database: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return 1;
    }
    return 0;
}

// Function to create an SQLite database at the specified path
/*
Accepts:
- char *db_path: The path where the SQLite database will be created.

Returns:
- sqlite3 *: A pointer to the SQLite database handle, or NULL on failure.
*/
sqlite3* create_database(const char *db_path) {
    sqlite3 *db;  // SQLite database handle

    // Open the SQLite database
    int rc = sqlite3_open(db_path, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    return db;
}
// Function to create an SQLite table for installed packages
/*
Accepts:
- sqlite3 *db: An SQLite database handle.

Returns:
- int: An integer indicating the result of the table creation.
  - 0: Success.
  - 1: Failure, SQL error.
*/
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

// Function to store data for an installed package in the database
/*
Accepts:
- sqlite3 *db: An SQLite database handle.
- struct package *pkg: A pointer to a package structure.
- int as_dep: Flag indicating whether the package is installed as a dependency (1 for yes, 0 for no).

Returns:
- int: An integer indicating the result of data storage.
  - 0: Success.
  - -1: Failure, SQL error.
*/
int store_data_installed(sqlite3 *db, struct package *pkg, int as_dep) {
    sqlite3_stmt *stmt;  // Statement handle.
    int rc;              // Return code.

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
    dbg(1, "Inserted %s %s %s -- exit: %d ", pkg->name, pkg->version, pkg->type, rc);

    // Finalize the statement.
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        msg(ERROR, "Error finalizing INSERT statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

// Function to retrieve data for an installed package from the database
/*
Accepts:
- sqlite3 *db: An SQLite database handle.
- struct package *pkg: A pointer to a package structure.
- int *as_dep: A pointer to an integer to store whether the package was installed as a dependency (optional).

Returns:
- int: An integer indicating the result of data retrieval.
  - 0: Success.
  - -1: Failure, SQL error.
*/
int retrieve_data_installed(sqlite3 *db, struct package *pkg, int *as_dep) {
    sqlite3_stmt *stmt;
    int rc;

    dbg(1, "Retrieving %s data from DB", pkg->name);

    // Prepare the SQL query
    const char *sql = "SELECT Version, Type, AsDep FROM Packages WHERE Name = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s -- %d\n", sqlite3_errmsg(db), rc);
        return -1;
    }

    // Bind the value for the parameter in the SQL query
    sqlite3_bind_text(stmt, 1, pkg->name, -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        pkg->version = strdup((char *)sqlite3_column_text(stmt, 0));
        pkg->type = strdup((char *)sqlite3_column_text(stmt, 1));
        if (as_dep != NULL) *as_dep = sqlite3_column_int(stmt, 2);
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        msg(ERROR, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

// Function to remove data for an installed package from the database
/*
Accepts:
- sqlite3 *db: An SQLite database handle.
- char *name: The name of the package to remove.

Returns:
- int: An integer indicating the result of data removal.
  - 0: Success.
  - 1: Failure, SQL error.
*/
int remove_data_installed(sqlite3 *db, char *name) {
    sqlite3_stmt *stmt;
    int rc;

    // Prepare the SQL query
    const char *sql = "DELETE FROM Packages WHERE Name = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s -- %d", sqlite3_errmsg(db), rc);
        return 1;
    }

    // Bind the value for the parameter in the SQL query
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

    // Execute the DELETE statement.
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        msg(ERROR, "Error executing DELETE statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Finalize the statement.
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        msg(ERROR, "Error finalizing DELETE statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

// Function to retrieve data for a package from the repository database
/*
Accepts:
- sqlite3 *db: An SQLite database handle.
- struct package *pkg: A pointer to a package structure.
- char **format: A pointer to store the package format (optional).
- char **section: A pointer to store the package section (optional).

Returns:
- int: An integer indicating the result of data retrieval.
  - 0: Success.
  - 1: Failure, SQL error.
*/
int retrieve_data_repo(sqlite3 *db, struct package *pkg, char **format, char **section) {
    sqlite3_stmt *stmt;
    int rc;

    dbg(1, "Retrieving %s data from repo DB", pkg->name);

    // Prepare the SQL query
    const char *sql = "SELECT Version, Type, Format, Section FROM Packages WHERE Name = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(db));
        return 1;
    }

    // Bind the value for the parameter in the SQL query
    sqlite3_bind_text(stmt, 1, pkg->name, -1, SQLITE_STATIC);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        pkg->version = strdup((char *)sqlite3_column_text(stmt, 0));
        pkg->type = strdup((char *)sqlite3_column_text(stmt, 1));
        if (format != NULL) (*format) = strdup((char *)sqlite3_column_text(stmt, 2));
        if (section != NULL) (*section) = strdup((char *)sqlite3_column_text(stmt, 3));
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

// Function to print all data from the database
/*
Accepts:
- sqlite3 *db: An SQLite database handle.

Returns:
- int: An integer indicating the result of data printing.
  - 0: Success.
  - 1: Failure, SQL error.
*/
int print_all_data(sqlite3 *db) {
    sqlite3_stmt *stmt;
    char *zErrMsg = 0;
    int rc;

    // Prepare the SQL query
    const char *sql = "SELECT Name, Version, Type FROM Packages";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        msg(ERROR, "SQL error: %s", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }

    // Execute the SQL query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        msg(INFO, "%s ==> %s - %s", sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2));
    }

    // Check if the SQL query was successful
    if (rc != SQLITE_DONE) {
        msg(ERROR, "SQL error: %s", sqlite3_errmsg(db));
        // could remove this _free() probably..
        sqlite3_free(zErrMsg);
        return -1;
    }

    return 0;
}
