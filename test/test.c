#include "test.h"

#include "../include/libspm.h"

void test_check() 
{
    setenv("SOVIET_SPM_DIR", "/tmp/cccp-test", 1);
    setenv("SOVIET_FORMATS", "ecmp", 1);
    setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 1);

    FILE *ptr;
    ptr = fopen("/tmp/cccp-test/test.ecmp","w"); 
    fprintf(ptr, "[locations]\n");
    fprintf(ptr, "/tmp/cccp-test/test.ecmp\n");
    fclose(ptr);

    struct package pkg = {0};
    pkg.path = "test.ecmp";
    int result = check(&pkg);
    free_pkg(&pkg);

    unsetenv("SOVIET_SPM_DIR");
    unsetenv("SOVIET_FORMATS");
    unsetenv("SOVIET_PLUGIN_DIR");
    rmany("/tmp/cccp-test/test.ecmp");

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");
}

void test_clean() 
{
    char TEST_BUILD_DIR[MAX_PATH];
    char TEST_MAKE_DIR[MAX_PATH];
    
    sprintf(TEST_BUILD_DIR, "%s/%s", getenv("SOVIET_TEST_DIR"), "TEST_BUILD_DIR");
    sprintf(TEST_MAKE_DIR, "%s/%s", getenv("SOVIET_TEST_DIR"), "TEST_MAKE_DIR");
    
    setenv("SOVIET_BUILD_DIR", TEST_BUILD_DIR, 1);
    setenv("SOVIET_MAKE_DIR", TEST_MAKE_DIR, 1);

    pmkdir(getenv("SOVIET_BUILD_DIR"));
    pmkdir(getenv("SOVIET_MAKE_DIR"));

    int result = clean();

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");

    rmany(getenv("SOVIET_BUILD_DIR"));
    rmany(getenv("SOVIET_MAKE_DIR"));
    unsetenv("SOVIET_BUILD_DIR");
    unsetenv("SOVIET_MAKE_DIR");
}

void test_config() 
{
    setenv("SOVIET_TEST_ENV", "TEST", 1);

    FILE *ptr;
    ptr = fopen("/tmp/cccp-test/test.conf","w"); 
    fprintf(ptr, "SOVIET_TEST_VAR=$SOVIET_TEST_ENV/TEST");
    fclose(ptr);

    int result = readConfig("/tmp/cccp-test/test.conf", 1);

    if((result != 0) || (strcmp(getenv("SOVIET_TEST_VAR"), "TEST/TEST") != 0))
    {
        msg(ERROR, "EXPECTED: TEST/TEST");
        msg(ERROR, "GOT: %s", getenv("SOVIET_TEST_VAR"));
        msg(FATAL, "FAILED");
    }
    else msg(INFO, "PASSED");

    unsetenv("SOVIET_TEST_ENV");
    unsetenv("SOVIET_TEST_VAR");
    rmany("/tmp/cccp-test/test.conf");
}

void test_install() 
{
    int result = 0;

    // Define variables used during installation
    {
        setenv("SOVIET_SPM_DIR", "/tmp/cccp-test/spm_dir", 1);
        setenv("SOVIET_SOURCE_DIR", "/tmp/cccp-test/src_dir", 1);
        
        pmkdir("/tmp/cccp-test/spm_dir");
        pmkdir("/tmp/cccp-test/src_dir");

        setenv("SOVIET_FORMATS", "ecmp", 1);
        setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 1);

        char TEST_BUILD_DIR[MAX_PATH];
        char TEST_MAKE_DIR[MAX_PATH];
        
        sprintf(TEST_BUILD_DIR, "%s/%s", getenv("SOVIET_TEST_DIR"), "TEST_BUILD_DIR");
        sprintf(TEST_MAKE_DIR, "%s/%s", getenv("SOVIET_TEST_DIR"), "TEST_MAKE_DIR");
        
        setenv("SOVIET_BUILD_DIR", TEST_BUILD_DIR, 1);
        setenv("SOVIET_MAKE_DIR", TEST_MAKE_DIR, 1);

        pmkdir(getenv("SOVIET_BUILD_DIR"));
        pmkdir(getenv("SOVIET_MAKE_DIR"));
        
    }
    // Create test package
    {
        FILE *ptr;
        ptr = fopen("/tmp/cccp-test/test.ecmp","w"); 
        fprintf(    ptr,
                    "[info]\n"                                                                                      \
                    "name = test\n"                                                                                 \
                    "version = 0\n"                                                                                 \
                    "type = src\n"                                                                                  \
                    "[files]\n"                                                                                     \
                    "$VERSION-file 127.0.0.1 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"    \
                    "[description]\n"                                                                               \
                    "test package\n"                                                                                \
                    "[prepare]\n"                                                                                   \
                    "mkdir test-0\n"                                                                                \
                    "mv 0-file ./test-0/0-file\n"                                                                   \
                    "[install]\n"                                                                                   \
                    "mv 0-file $SOVIET_BUILD_DIR/0-file\n"                                                          \
                    "[special]\n"                                                                                   \
                    "echo special...\n"
                );

        fclose(ptr);
    }

    struct package pkg = {0};
    pkg.name = "test";
    pkg.path = "test.ecmp";
    pkg.format = "ecmp";
    
    result += open_pkg("/tmp/cccp-test/test.ecmp", &pkg);
    result += install_package_source(&pkg);

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");

    // Cleanup
    {
        free_pkg(&pkg);
        rmany(getenv("SOVIET_BUILD_DIR"));
        rmany(getenv("SOVIET_MAKE_DIR"));
        rmany(getenv("SOVIET_SPM_DIR"));
        rmany(getenv("SOVIET_SOURCE_DIR"));
        unsetenv("SOVIET_BUILD_DIR");
        unsetenv("SOVIET_MAKE_DIR");    
        unsetenv("SOVIET_SPM_DIR");
        unsetenv("SOVIET_FORMATS");
        unsetenv("SOVIET_PLUGIN_DIR");
        rmany("/tmp/cccp-test/test.ecmp");
    }
}

void test_make() 
{
    return;
}

void test_move() 
{
    return;
}

void test_pkg() 
{
    return;

}

void test_repo()
{
    return;
}

void test_uninstall()
{
    return;
}

void test_update()
{
    return;
}

void test_util()
{
    return;
}
