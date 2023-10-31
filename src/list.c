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
        msg(INFO, "listing installed packages from %s", getenv(INSTALLED_DB));
        if(0 == 1)
            {
                sqlite3_stmt *stmt;
                int rc;

                // Prepare the SQL query
                const char *sql = "idfk";
                rc = sqlite3_prepare_v2(getenv(INSTALLED_DB), sql, -1, &stmt, NULL);
                if (rc != SQLITE_OK) {
                    msg(ERROR, "SQL error: %s -- %d", sqlite3_errmsg(INSTALLED_DB), rc);
                    return 1;
                }

                // Bind the value for the parameter in the SQL query
                sqlite3_bind_text(stmt, 1, "idk", -1, SQLITE_STATIC);

                // Execute the PRINT statement or smth.
                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE) {
                    msg(ERROR, "Error executing PRINT statement or smth: %s\n", sqlite3_errmsg(INSTALLED_DB));
                    return -1;
                }

                // Finalize the statement.
                rc = sqlite3_finalize(stmt);
                if (rc != SQLITE_OK) {
                    msg(ERROR, "Error finalizing PRINT statement or smth: %s\n", sqlite3_errmsg(INSTALLED_DB));
                    return -1;
            }

        return 0;
        }

    }
    
//will return the ammount of packages installed  INSTALLED_DB
int count_installed()
    {
        int count = 1;
        
        // do sql magic here
        
        msg(INFO, "there are %d installed packages", count);
        
        return count;
    }

//will print the content of INSTALLED_DB at a specific index
int get_installed(int index)
    {
        int count = 0;
        char* out = "test";
        //out = sql magic
        
        msg(INFO, "%s is installed at %d", out, index);
        
        return count;

    }
