#include "test.h"
#include <threads.h>

char WORKING_DIR[2048];

int main(int argc, char const *argv[])
{
    msg(INFO,"Started LibSPM test suite...");
    if (argc  < 2)
    {
        printf("No arguments provided\n");
        return 1;
    }

    /* START LIBSPM CONFIG */
    setenv("SOVIET_DEBUG","3",1);
    DEBUG = 3;
    QUIET = false;
    OVERWRITE = true;
    DEBUG_UNIT = NULL;
    // we want to chnage that later 
    // TODO: Add hash to test package
    INSECURE = true;
    /* END LIBSPM CONFIG */

    char cwd[2048];
    getcwd(cwd, 2048);
    sprintf(WORKING_DIR,"%s/test/assets",cwd);

    char TEST_SPM_PATH[2048];
    sprintf(TEST_SPM_PATH,"%s/vim.ecmp",WORKING_DIR);



   if (argc < 2 || strcmp(argv[1], "help") == 0) {
        printf("Usage: %s [ecmp|all|make|install|uninstall|move|help|split|config|get]\n", argv[0]);
        return 0;
    }

    // Check for root privileges for all other commands
    if (geteuid() != 0) {
        printf("You must have root privileges to run this command.\n");
        return 1;
    }

    if (strcmp(argv[1], "all") == 0) {
        test_move();
        test_get();
        test_split();
        test_config();

        test_ecmp(TEST_SPM_PATH);
        test_make(TEST_SPM_PATH);
        test_pm(TEST_SPM_PATH);

        int leaks = check_leaks();
        if (leaks > 0) {
            msg(ERROR, "Leaks: %d",leaks);
        }

    } else if (strcmp(argv[1], "ecmp") == 0) {
        test_ecmp(TEST_SPM_PATH);
    }  else if (strcmp(argv[1], "make") == 0) {
        test_make(TEST_SPM_PATH);
        printf("Leaks: %d\n", check_leaks());
    } else if (strcmp(argv[1],"install") == 0 || 
               strcmp(argv[1],"pm") == 0) {
        test_pm(TEST_SPM_PATH);
        printf("Leaks: %d\n", check_leaks());
        return 0;
    } else if (strcmp(argv[1], "move") == 0) {
        test_move();
    } else if (strcmp(argv[1], "split") == 0) {
        test_split();
    } else if (strcmp(argv[1], "config") == 0) {
        test_config();
    } else if (strcmp(argv[1], "get") == 0) {
        test_get();
    } else {
        printf("Invalid argument\n");
        return 1;
    }

    int leaks = check_leaks();
    if (leaks > 0) {
        msg(WARNING,"Leaks: %d",leaks);
    }

    msg(INFO,"Done testing LibSPM.");
    return 0;
}
