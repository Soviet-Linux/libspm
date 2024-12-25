#include "test.h"

#include "../include/libspm.h"
#include <git2.h>

void test_check() 
{
    // Necessary prepwork
    {
        setenv("SOVIET_SPM_DIR", "/tmp/cccp-test", 1);
        setenv("SOVIET_FORMATS", "ecmp", 1);
        setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 1);

        // I'm heavily abusing the fact that there is no check on the validity of the package
        FILE *ptr;
        ptr = fopen("/tmp/cccp-test/test.ecmp","w"); 
        fprintf(ptr, "[locations]\n");
        fprintf(ptr, "/tmp/cccp-test/test.ecmp\n");
        fclose(ptr);
    }

    struct package pkg = {0};
    pkg.path = strdup("test.ecmp");
    int result = check(&pkg);
    free_pkg(&pkg);

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");

    // Cleanup
    {
        unsetenv("SOVIET_SPM_DIR");
        unsetenv("SOVIET_FORMATS");
        unsetenv("SOVIET_PLUGIN_DIR");
        rmany("/tmp/cccp-test/test.ecmp");
    }
}

void test_clean() 
{
    // Necessary prepwork
    {
        char TEST_BUILD_DIR[MAX_PATH];
        char TEST_MAKE_DIR[MAX_PATH];
        
        sprintf(TEST_BUILD_DIR, "%s/%s", getenv("SOVIET_TEST_DIR"), "TEST_BUILD_DIR");
        sprintf(TEST_MAKE_DIR, "%s/%s", getenv("SOVIET_TEST_DIR"), "TEST_MAKE_DIR");
        
        setenv("SOVIET_BUILD_DIR", TEST_BUILD_DIR, 1);
        setenv("SOVIET_MAKE_DIR", TEST_MAKE_DIR, 1);

        pmkdir(getenv("SOVIET_BUILD_DIR"));
        pmkdir(getenv("SOVIET_MAKE_DIR"));
    }

    int result = clean();

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");

    // Cleanup
    {
        rmany(getenv("SOVIET_BUILD_DIR"));
        rmany(getenv("SOVIET_MAKE_DIR"));
        unsetenv("SOVIET_BUILD_DIR");
        unsetenv("SOVIET_MAKE_DIR");
    }
}

void test_config() 
{
    // Necessary prepwork
    {
        setenv("SOVIET_TEST_ENV", "TEST", 1);

        FILE *ptr;
        ptr = fopen("/tmp/cccp-test/test.conf","w"); 
        fprintf(ptr, "SOVIET_TEST_VAR=$SOVIET_TEST_ENV/TEST");
        fclose(ptr);
    }

    int result = readConfig("/tmp/cccp-test/test.conf", 1);

    if((result != 0) || (strcmp(getenv("SOVIET_TEST_VAR"), "TEST/TEST") != 0))
    {
        msg(ERROR, "EXPECTED: 'TEST/TEST'");
        msg(ERROR, "GOT: '%s'", getenv("SOVIET_TEST_VAR"));
        msg(FATAL, "FAILED");
    }
    else msg(INFO, "PASSED");
    
    // Cleanup
    {
        unsetenv("SOVIET_TEST_ENV");
        unsetenv("SOVIET_TEST_VAR");
        rmany("/tmp/cccp-test/test.conf");
    }
}

void test_install() 
{
    int result = 0;

    // Necessary prepwork
    {
        setenv("SOVIET_SPM_DIR", "/tmp/cccp-test/spm_dir", 1);
        setenv("SOVIET_SOURCE_DIR", "/tmp/cccp-test/src_dir", 1);
        setenv("SOVIET_ROOT", "/tmp/cccp-test/destination/", 1);
        setenv("SOVIET_ENV_DIR", "/tmp/cccp-test/src_dir", 1);
        setenv("SOVIET_TEST_ENV", "TEST", 1);
        
        pmkdir(getenv("SOVIET_ROOT"));
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
        pmkdir(getenv("SOVIET_ENV_DIR"));
        
        FILE *ptr;
        ptr = fopen("/tmp/cccp-test/test.ecmp","w"); 
        fprintf(    ptr,
                    "[info]\n"                                                                                      
                    "name = test\n"                                                                                 
                    "version = 0\n"                                                                                 
                    "type = src\n"                                                                                  
                    "[files]\n"                                                                                     
                    "$VERSION-file 127.0.0.1 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"    
                    "[description]\n"                                                                               
                    "test package\n"                                                                                
                    "[config]\n"                                                                                    
                    "SOVIET_TEST_VAR=$SOVIET_TEST_ENV\n"                                                                                
                    "[prepare]\n"                                                                                   
                    "mkdir test-0\n"                                                                                
                    "mv 0-file ./test-0/0-file\n"                                                                   
                    "[install]\n"                                                                                   
                    "mv 0-file $SOVIET_BUILD_DIR/0-file\n"                                                          
                    "[special]\n"                                                                                   
                    "echo special...\n"
                );

        fclose(ptr);
    }

    struct package pkg = {0};
    pkg.name = strdup("test");
    pkg.path = strdup("test.ecmp");
    
    result += open_pkg("/tmp/cccp-test", &pkg);
    write_package_configuration_file(&pkg);
    read_package_configuration_file(&pkg);
    result += strcmp(getenv("SOVIET_TEST_VAR"), "TEST");
    if(result != 0)
    {
        msg(ERROR, "EXPECTED: 'TEST'");
        msg(ERROR, "GOT: '%s'", getenv("SOVIET_TEST_VAR"));
        msg(FATAL, "FAILED");        
    }
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
        unsetenv("SOVIET_TEST_ENV");
        unsetenv("SOVIET_TEST_VAR");
        rmany("/tmp/cccp-test/test.ecmp");
    }
}

void test_make() 
{
    int result = 0;

    // Necessary prepwork
    {
        setenv("SOVIET_SOURCE_DIR", "/tmp/cccp-test/src_dir", 1);
        pmkdir("/tmp/cccp-test/src_dir");

        setenv("SOVIET_FORMATS", "ecmp", 1);
        setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 1);

        char TEST_MAKE_DIR[MAX_PATH];
        sprintf(TEST_MAKE_DIR, "%s/%s", getenv("SOVIET_TEST_DIR"), "TEST_MAKE_DIR");
        setenv("SOVIET_MAKE_DIR", TEST_MAKE_DIR, 1);
        pmkdir(getenv("SOVIET_MAKE_DIR"));
        
        FILE *ptr;
        ptr = fopen("/tmp/cccp-test/test.ecmp","w"); 
        fprintf(    ptr,
                    "[info]\n"                                                                                      
                    "name = test\n"                                                                                 
                    "version = 0\n"                                                                                 
                    "type = src\n"                                                                                  
                    "[files]\n"                                                                                     
                    "$VERSION-file 127.0.0.1 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"               
                    "[prepare]\n"                                                                                   
                    "mkdir test-0\n"                                                                                
                    "mv 0-file ./test-0/0-file\n"                                                                   
                );

        fclose(ptr);
    }

    struct package pkg = {0};
    pkg.name = strdup("test");
    pkg.path = strdup("test.ecmp");
    
    result += open_pkg("/tmp/cccp-test", &pkg);
    result += make(&pkg);
    result += access("/tmp/cccp-test/TEST_MAKE_DIR/test-0/0-file", F_OK);

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");

    // Cleanup
    {
        free_pkg(&pkg);
        rmany(getenv("SOVIET_MAKE_DIR"));
        rmany(getenv("SOVIET_SOURCE_DIR"));
        unsetenv("SOVIET_MAKE_DIR");    
        unsetenv("SOVIET_FORMATS");
        unsetenv("SOVIET_PLUGIN_DIR");
        rmany("/tmp/cccp-test/test.ecmp");
    }
}

void test_move() 
{
    // Necessary prepwork
    {
        setenv("SOVIET_BUILD_DIR", "/tmp/cccp-test/source/", 1);
        setenv("SOVIET_ROOT", "/tmp/cccp-test/destination/", 1);
        pmkdir(getenv("SOVIET_BUILD_DIR"));
        pmkdir(getenv("SOVIET_ROOT"));
    
        FILE *ptr;
        ptr = fopen("/tmp/cccp-test/source/test","w"); 
        fprintf(ptr, "test file");
        fclose(ptr);
    }

    int num_files;
    char **files_array = get_all_files(getenv("SOVIET_BUILD_DIR"), getenv("SOVIET_BUILD_DIR"), &num_files);
    move_binaries(files_array, num_files);

    int result = access("/tmp/cccp-test/destination/test", F_OK);

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");

    // Cleanup 
    {
        for(int i = 0; i < num_files; i++)
        {
            free(files_array[i]);
        }
        free(files_array);
        rmany(getenv("SOVIET_BUILD_DIR"));
        rmany(getenv("SOVIET_ROOT"));
        unsetenv("SOVIET_ROOT");
        unsetenv("SOVIET_ROOT");
    }
}

void test_pkg() 
{
    int result = 0;
    // Test package array
    {
        struct packages* pkgs = create_pkgs(0); 
        for(int i = 0; i < 5; i++)
        {
            struct package pkg = {0};
            pkg.name = strdup("test_part_1");
            pkg.description = strdup("test description");
            push_pkg(pkgs, &pkg);
        }
        result += (pkgs->count - 5);
        dbg(2, "size %d - count %d", pkgs->size, pkgs->count);
        
        struct packages* t_pkgs = create_pkgs(0);
        for(int i = 0; i < 5; i++)
        {
            struct package pkg = {0};
            pkg.name = strdup("test_part_2");
            pkg.description = strdup("test description");
            push_pkg(t_pkgs, &pkg);
        }
        result += (t_pkgs->count - 5);
        dbg(2, "size %d - count %d", t_pkgs->size, t_pkgs->count);

        merge_pkgs(pkgs, t_pkgs);
        result += (pkgs->count - 10);
        dbg(2, "size %d - count %d", pkgs->size, pkgs->count);

        dbg(2, "result: %d ", result);
        // Cleanup
        {
            free_pkgs(pkgs);    
        }
    }

    // Test create and open package
    {
        // Necessary prepwork
        {
            setenv("SOVIET_SPM_DIR", "/tmp/cccp-test/spm_dir", 1);
            pmkdir("/tmp/cccp-test/spm_dir");

            setenv("SOVIET_FORMATS", "ecmp", 1);
            setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 1);
            
            FILE *ptr;
            ptr = fopen("/tmp/cccp-test/test.ecmp","w"); 
            fprintf(    ptr,
                        "[info]\n"                                                                                      
                        "name = test\n"                                                                                 
                        "version = 0\n"                                                                                 
                        "type = src\n"                                                                                  
                        "[files]\n"                                                                                     
                        "$VERSION-file 127.0.0.1 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n"    
                        "[description]\n"                                                                               
                        "test package\n"                                                                                
                        "[config]\n"                                                                                    
                        "SOVIET_TEST_VAR=$SOVIET_TEST_ENV\n"                                                                                
                        "[prepare]\n"                                                                                   
                        "mkdir test-0\n"                                                                                
                        "mv 0-file ./test-0/0-file\n"                                                                   
                        "[install]\n"                                                                                   
                        "mv 0-file $SOVIET_BUILD_DIR/0-file\n"                                                          
                        "[special]\n"                                                                                   
                        "echo special...\n"
                    );

            fclose(ptr);
        }

        struct package pkg_0 = {0};
        pkg_0.path = "test-0.ecmp";
        pkg_0.name = "test";
        pkg_0.version = "0";
        pkg_0.type = "src";
        pkg_0.description= "test package\n";
        result += create_pkg("/tmp/cccp-test", &pkg_0);

        struct package pkg_1 = {0};
        pkg_1.name = strdup("test");
        pkg_1.path = strdup("test-0.ecmp");
        result += open_pkg("/tmp/cccp-test", &pkg_1);
        result += strcmp(pkg_1.name, "test");
        result += strcmp(pkg_1.version, "0");
        result += strcmp(pkg_1.type, "src");
        result += strcmp(pkg_1.description, "test package\n");
        dbg(2, "Finished testing create and open package");
        dbg(2, "result: %d ", result);

        // Cleanup
        {
            free_pkg(&pkg_1);
            rmany(getenv("SOVIET_SPM_DIR"));
            unsetenv("SOVIET_SPM_DIR");
            unsetenv("SOVIET_FORMATS");
            unsetenv("SOVIET_PLUGIN_DIR");
            rmany("/tmp/cccp-test/test.ecmp");
            rmany("/tmp/cccp-test/test-0.ecmp");
        }
    }

    // Test package database
    {
        // Necessary prepwork
        {
            setenv("SOVIET_REPOS_DIR", "/tmp/cccp-test/repo_dir", 1);
            setenv("SOVIET_SPM_DIR", "/tmp/cccp-test/spm_dir", 1);
            setenv("SOVIET_DEFAULT_FORMAT", "ecmp", 1);

            pmkdir(getenv("SOVIET_REPOS_DIR"));
            pmkdir("/tmp/cccp-test/repo_dir/test_repo/long/repo/file/tree");
            pmkdir(getenv("SOVIET_SPM_DIR"));
            for(int i = 0; i < 10; i++)
            {
                char dir[MAX_PATH];
                sprintf(dir, "%s/%s/%d%s", getenv("SOVIET_REPOS_DIR"), "test_repo/long/repo/file/tree", i, ".ecmp");

                FILE *ptr;
                ptr = fopen(dir,"w"); 
                fprintf(ptr, "test file");
                fclose(ptr);
            }
            // Garbage files
            for(int i = 0; i < 10; i++)
            {
                char dir[MAX_PATH];
                sprintf(dir, "%s/%s/%s-%d", getenv("SOVIET_REPOS_DIR"), "test_repo/long/repo/file/tree", ".ecmp", i);

                FILE *ptr;
                ptr = fopen(dir,"w"); 
                fprintf(ptr, "test file");
                fclose(ptr);
            }
        }

        // Get packages
        struct packages* pkgs = get_pkgs("/tmp/cccp-test/repo_dir/");
        // there should be 10 packages in that directory
        result += (pkgs->count - 10);
        for(int i = 0; i < 10; i++)
        {
            char str[8];
            sprintf(str, "%d", i);
            // all the found packages should be 0-9
            result += strcmp(pkgs->buffer[i].name, str);
        }
        
        // Create a database
        result += create_pkg_db("/tmp/cccp-test/test.db", pkgs);

        // Preform a search
        struct packages* search_result = search_pkgs("/tmp/cccp-test/test.db", "1");
        // we should find only one '1'
        result += (search_result->count - 1);
        result += strcmp(search_result->buffer[0].path, "test_repo/long/repo/file/tree/1.ecmp");

        struct packages* db_pkgs = dump_db("/tmp/cccp-test/test.db");
        // there should be 10 packages in the database
        result += (db_pkgs->count - 10);
        dbg(2, "Finished testing package database");

        // Cleanup
        {
            free_pkgs(pkgs);
            free_pkgs(db_pkgs);
            free_pkgs(search_result);
            rmany(getenv("SOVIET_REPOS_DIR"));
            rmany(getenv("SOVIET_SPM_DIR"));
            rmany("/tmp/cccp-test/test.db");
            unsetenv("SOVIET_REPOS_DIR");
            unsetenv("SOVIET_DEFAULT_FORMAT");
            unsetenv("SOVIET_SPM_DIR");
        }
    }

    // Test updating packages
    {
        // Necessary prepwork
        {
            setenv("SOVIET_REPOS_DIR", "/tmp/cccp-test/repo_dir", 1);
            setenv("SOVIET_SPM_DIR", "/tmp/cccp-test/spm_dir", 1);
            setenv("SOVIET_DEFAULT_FORMAT", "ecmp", 1);
            setenv("SOVIET_FORMATS", "ecmp", 1);
            setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 1);

            pmkdir(getenv("SOVIET_REPOS_DIR"));
            pmkdir("/tmp/cccp-test/repo_dir/test_repo/long/repo/file/tree");

            pmkdir(getenv("SOVIET_SPM_DIR"));
            pmkdir("/tmp/cccp-test/spm_dir/test_repo/long/repo/file/tree");

            // File 1 - outdated
            {
                char spm_dir[MAX_PATH];
                char repo_dir[MAX_PATH];
                sprintf(repo_dir, "%s/%s/%s", getenv("SOVIET_REPOS_DIR"), "test_repo/long/repo/file/tree", "outdated.ecmp");
                sprintf(spm_dir, "%s/%s/%s", getenv("SOVIET_SPM_DIR"), "test_repo/long/repo/file/tree", "outdated.ecmp");

                FILE *ptr;
                ptr = fopen(repo_dir,"w"); 
                fprintf(ptr, "[info]\n");
                fprintf(ptr, "name = outdated\n");
                fprintf(ptr, "version = 1\n");
                fclose(ptr);

                ptr = fopen(spm_dir,"w"); 
                fprintf(ptr, "[info]\n");
                fprintf(ptr, "name = outdated\n");
                fprintf(ptr, "version = 0\n");
                fclose(ptr);
            }

            // File 2 - same
            {
                char spm_dir[MAX_PATH];
                char repo_dir[MAX_PATH];
                sprintf(repo_dir, "%s/%s/%s", getenv("SOVIET_REPOS_DIR"), "test_repo/long/repo/file/tree", "same.ecmp");
                sprintf(spm_dir, "%s/%s/%s", getenv("SOVIET_SPM_DIR"), "test_repo/long/repo/file/tree", "same.ecmp");

                FILE *ptr;
                ptr = fopen(repo_dir,"w"); 
                fprintf(ptr, "[info]\n");
                fprintf(ptr, "name = same\n");
                fprintf(ptr, "version = 1\n");
                fclose(ptr);

                ptr = fopen(spm_dir,"w"); 
                fprintf(ptr, "[info]\n");
                fprintf(ptr, "name = same\n");
                fprintf(ptr, "version = 1\n");
                fclose(ptr);
            }

            // File 3 - local only
            {
                char spm_dir[MAX_PATH];
                sprintf(spm_dir, "%s/%s/%s", getenv("SOVIET_SPM_DIR"), "test_repo/long/repo/file/tree", "local.ecmp");

                FILE *ptr;
                ptr = fopen(spm_dir,"w"); 
                fprintf(ptr, "[info]\n");
                fprintf(ptr, "name = local\n");
                fprintf(ptr, "version = 1\n");
                fclose(ptr);
            }

            struct packages* installed = get_pkgs(getenv("SOVIET_SPM_DIR"));
            struct packages* remote = get_pkgs(getenv("SOVIET_REPOS_DIR"));

            setenv("SOVIET_ALL_DB", "/tmp/cccp-test/all.db", 1);
            setenv("SOVIET_INSTALLED_DB", "/tmp/cccp-test/installed.db", 1);

            create_pkg_db(getenv("SOVIET_ALL_DB"), remote);
            create_pkg_db(getenv("SOVIET_INSTALLED_DB"), installed);

            free_pkgs(installed);
            free_pkgs(remote);
        }
    
        struct packages* need_updating = update_pkg();
        // We expect only one of the packages to need updating
        result += (need_updating->count - 1);
        dbg(2, "Finished testing package update");

        result += strcmp(need_updating->buffer[0].name, "outdated");
        result += strcmp(need_updating->buffer[0].version, "1");

        // Cleanup
        {
            free_pkgs(need_updating);
            
            rmany(getenv("SOVIET_REPOS_DIR"));
            rmany(getenv("SOVIET_SPM_DIR"));
            rmany(getenv("SOVIET_ALL_DB"));
            rmany(getenv("SOVIET_INSTALLED_DB"));

            unsetenv("SOVIET_REPOS_DIR");
            unsetenv("SOVIET_SPM_DIR");
            unsetenv("SOVIET_DEFAULT_FORMAT");
            unsetenv("SOVIET_FORMATS");
            unsetenv("SOVIET_PLUGIN_DIR");
            unsetenv("SOVIET_ALL_DB");
            unsetenv("SOVIET_INSTALLED_DB");
        }
    }

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");
}

void test_repo()
{
    int result = 0;
    // Necessary prepwork
    {
        setenv("SOVIET_REPOS_DIR", "/tmp/cccp-test/repo_dir", 1);
        setenv("SOVIET_DEFAULT_REPO_URL", "https://github.com/Soviet-Linux/OUR.git", 1);
        setenv("SOVIET_DEFAULT_REPO", "OUR", 1);
        setenv("SOVIET_DEFAULT_FORMAT", "ecmp", 1);

        pmkdir("/tmp/cccp-test/repo_dir/repo-1");
        pmkdir("/tmp/cccp-test/repo_dir/repo-2");
        pmkdir("/tmp/cccp-test/repo_dir/repo-3");

        git_libgit2_init();
    }

    int repo_count;
    char** REPOS = get_repos(&repo_count);
    // we expect 3 repos + 1 local;
    result += (repo_count - 4);
    result += repo_sync();

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");

    // Cleanup
    {
        git_libgit2_shutdown();

        for(int i = 0; i < 4; i++)
        {
            free(REPOS[i]);
        }
        free(REPOS);
        rmany("/tmp/cccp-test/repo_dir");
        unsetenv("SOVIET_REPOS_DIR");
        unsetenv("SOVIET_DEFAULT_REPO_URL");
        unsetenv("SOVIET_DEFAULT_REPO");
        unsetenv("SOVIET_DEFAULT_FORMAT");
    }   
}

void test_uninstall()
{
    // Necessary prepwork
    {
        setenv("SOVIET_SPM_DIR", "/tmp/cccp-test", 1);
        setenv("SOVIET_FORMATS", "ecmp", 1);
        setenv("SOVIET_PLUGIN_DIR", "/var/cccp/plugins", 1);
        setenv("SOVIET_ROOT", "/", 1);

        FILE *ptr;
        ptr = fopen("/tmp/cccp-test/test.ecmp","w"); 
        fprintf(ptr, "[locations]\n");
        fprintf(ptr, "/tmp/cccp-test/test.ecmp\n");
        fclose(ptr);
    }

    struct package pkg = {0};
    pkg.path = strdup("test.ecmp");
    int result = uninstall(&pkg);
    free_pkg(&pkg);

    if(result != 0) msg(FATAL, "FAILED");
    else            msg(INFO, "PASSED");

    // Cleanup
    {
        unsetenv("SOVIET_SPM_DIR");
        unsetenv("SOVIET_ROOT");
        unsetenv("SOVIET_FORMATS");
        unsetenv("SOVIET_PLUGIN_DIR");
    }
}