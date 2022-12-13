#include <sqlite3.h>

// create the database and the table
int connect_db(sqlite3 **db,char* DB_PATH);
int create_table(sqlite3 *db);

int retrieve_data_installed (sqlite3 *db, struct package *pkg,int* as_dep);
int store_data_installed (sqlite3 *db, struct package *pkg,int as_dep);
int remove_data_installed (sqlite3* db,char* name);

int retrieve_data_repo(sqlite3 *db, struct package *pkg,char** format);

// list everything in the database
int print_all_data(sqlite3 *db);


