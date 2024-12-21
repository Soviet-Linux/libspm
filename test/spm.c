// Tests disabled for now
#if 1
#include "test.h"
#include <threads.h>

char WORKING_DIR[2048];

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

    char cwd[2048];
    getcwd(cwd, 2048);
    sprintf(WORKING_DIR,"%s/test/assets",cwd);

    char TEST_SPM_PATH[2048];
    sprintf(TEST_SPM_PATH,"%s/vim.ecmp",WORKING_DIR);

    // Check for root privileges for all other commands
    if (geteuid() != 0) {
        printf("You must have root privileges to run this command.\n");
        return 1;
    }

    test_check();
    test_clean();
    test_config();
    test_globals();
    test_init();
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

    msg(INFO,"Done testing LibSPM.");
    return 0;
}