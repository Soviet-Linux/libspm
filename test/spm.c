#include "test.h"
#include "../include/libspm.h"
#include <threads.h>

int main()
{
    msg(INFO,"Started LibSPM test suite...");

    /* START LIBSPM CONFIG */
    setenv("SOVIET_DEBUG","3",1);
    DEBUG = 3;
    QUIET = false;
    OVERWRITE = true;
    DEBUG_UNIT = NULL;
    /* END LIBSPM CONFIG */

    // Check for root privileges for all other commands
    if (geteuid() != 0) {
        printf("You must have root privileges to run this command.\n");
        return 1;
    }

    setenv("SOVIET_TEST_DIR", "/tmp/cccp-test", 1);
    rmany(getenv("SOVIET_TEST_DIR"));
    pmkdir(getenv("SOVIET_TEST_DIR"));

    test_check();
    test_clean();
    test_config();
    test_install();
    test_make();
    test_move();
    test_pkg();
    test_repo();
    test_uninstall();
    test_update();
    test_util();

    int leaks = check_leaks();
    if (leaks > 0) 
    {
        msg(WARNING,"Leaks: %d",leaks);
    }
    else
    {
        msg(INFO, "No leaks detected ;3");
    }

    msg(INFO,"Done testing LibSPM.");
    return 0;
}