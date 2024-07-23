#include "stdio.h"
#include "assert.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

#include "../include/globals.h"
#include "../include/libspm.h"
#include "../include/cutils.h"
#include "../include/libspm.h"


#define STATIC


extern int open_ecmp(char* path,struct package* pkg);
extern int create_ecmp(char* path,struct package* pkg);


char** list_of_stuff  = NULL;
int list_of_stuff_count = 0;

void test_ecmp();
void test_move();
void test_get();
void test_split();
void test_config();
void test_make();

void test_pm();

char* assemble(char** list,int count);

char CURRENT_DIR[2048];
char TEST_SPM_PATH[2048];

bool OVERWRITE;

int main(int argc, char const *argv[])
{
    msg(INFO,"Started LibSPM test suite...");
    if (argc  < 2)
    {
        printf("No arguments provided\n");
        return 1;
    }

    setenv("SOVIET_DEBUG","3",1);
    DEBUG = 3;
    QUIET = false;
    OVERWRITE = true;
    DEBUG_UNIT = NULL;
    // we want to chnage that later 
    // TODO: Add hash to test package
    INSECURE = true;

    getcwd(CURRENT_DIR, 2048);
    sprintf(TEST_SPM_PATH,"%s/dev/vim.ecmp",CURRENT_DIR);



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
        init();
        test_ecmp();
        test_move();
        test_get();
        test_split();
        test_config();
        test_make();

        test_pm();
        int leaks = check_leaks();
        if (leaks > 0) {
            msg(ERROR, "Leaks: %d",leaks);
        }

    } else if (strcmp(argv[1], "ecmp") == 0) {
        test_ecmp();
    }  else if (strcmp(argv[1], "make") == 0) {
        test_make();
        printf("Leaks: %d\n", check_leaks());
    } else if (strcmp(argv[1],"install") == 0 || 
               strcmp(argv[1],"pm") == 0) {
        test_pm();
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

void test_pm() {
    // install tests
    char temp_dir_template[] = "/tmp/test_dir_XXXXXX";
    char* test_dir = mkdtemp(temp_dir_template);
    setenv("SOVIET_ROOT",test_dir,1);
    char test_spm_dir[2048];
    sprintf(test_spm_dir,"%s/spm/",test_dir);
    setenv("SOVIET_SPM_DIR",test_spm_dir,1);

    init() ;

    assert(install_package_source(TEST_SPM_PATH,0) == 0);
    assert(uninstall("vim") == 0);

    unsetenv("SOVIET_ROOT");    
    unsetenv("SOVIET_ROOT");

    return;
}




void test_move() {


    char temp_dir_template[] = "/tmp/test_dir_XXXXXX";
    char* test_dir = mkdtemp(temp_dir_template);
    char build_dir[2048];
    sprintf(build_dir,"%s/build",test_dir);

    #define l_d_count 5
    #define l_f_count 12
    #define l_l_count 3
    char* l_dirs[l_d_count] = {"b","b/d","s","s/j","s/j/k"};
    char* l_files[l_f_count] = {"w","b/d/e","a","d","b/y","b/c","b/f","s/j/k/z","s/j/k/x","s/j/k/c","s/j/k/v","s/j/k/b"};
    char* l_links[l_l_count][2] = {{"b/d/e","b/e"},{"s/j/k/z","s/z"},{"s/j/k/x","s/x"}};

    printf("Testing move\n");
    setenv("SOVIET_ROOT",test_dir,1);
    setenv("SOVIET_BUILD_DIR",build_dir,1);
    init();

    printf("Creating test dirs\n");
    //make all dirs
    for (int i = 0; i < l_d_count; i++)
    {
        printf("Creating %s\n",l_dirs[i]);
        char* dir = malloc(256);
        sprintf(dir,"%s/%s",build_dir,l_dirs[i]);
        mkdir(dir,0777);
        free(dir);
    }
    printf("creating test files\n");
    // make all files
    for (int i = 0; i < l_f_count; i++)
    {
        printf("Creating %s\n",l_files[i]);
        char* path = malloc(256);
        sprintf(path,"%s/%s",build_dir,l_files[i]);
        printf("Path: %s\n",path);
        FILE* f = fopen(path,"w");
        fclose(f);
        free(path);
    }
    printf("Creating test links\n");
    // make all links
    for (int i = 0; i < l_l_count; i++)
    {
        printf("Creating %s -> %s\n",l_links[i][1],l_links[i][0]);
        char* old_path = malloc(256);
        sprintf(old_path,"%s/%s",build_dir,l_links[i][0]);
        char* link_path = malloc(256);
        sprintf(link_path,"%s/%s",build_dir,l_links[i][1]);
        symlink(old_path,link_path);
        free(old_path);
        free(link_path);
    }

    // get all the files with get_locatins
    char** end_locations;
    int end_count = get_locations(&end_locations,build_dir);

    assert(end_count == l_f_count + l_l_count);




    move_binaries(end_locations,end_count);
    // Check if the move was successful
    int EXIT = 0;
    for (int i = 0; i < l_f_count; i++)
    {
        char* old_path = malloc(256);
        sprintf(old_path,"%s/%s",build_dir,l_files[i]);
        char* new_path = malloc(256);
        sprintf(new_path,"%s/%s",test_dir,l_files[i]);

        assert(access(old_path, F_OK) == -1 && access(new_path, F_OK) == 0);

        free(old_path);
        free(new_path);
    }
    // check if the links were moved
    for (int i = 0; i < l_l_count; i++)
    {
        char* old_path = malloc(256);
        sprintf(old_path,"%s/%s",build_dir,l_links[i][0]);
        char* new_path = malloc(256);
        sprintf(new_path,"%s/%s",test_dir,l_links[i][1]);

        // check using stat
        struct stat st;
        assert(stat(new_path,&st) == 0 && stat(old_path,&st) != 0);

        free(old_path);
        free(new_path);
    }


    free(*end_locations);
    free(end_locations);

    unsetenv("SOVIET_ROOT_DIR");
    unsetenv("SOVIET_BUILD_DIR");

}

void test_make() {

    init();
    struct package p = {0};

    assert(open_pkg(TEST_SPM_PATH, &p,NULL) == 0);

    printf("Testing make\n");

    char* legacy_dir = calloc(2048,1);
    sprintf(legacy_dir,"%s/%s-%s",getenv("SOVIET_MAKE_DIR"),p.name,p.version);

    dbg(1,"Legacy dir: %s",legacy_dir);

    assert(make(legacy_dir,&p) == 0);

    dbg(1,"Getting locations for %s",p.name);
    p.locationsCount = get_locations(&p.locations,getenv("SOVIET_BUILD_DIR"));
    assert(p.locationsCount > 0);

    dbg(1,"Got %d locations for %s",p.locationsCount,p.name);

    return;
}

void test_split() {   
    chdir(CURRENT_DIR);
    system("python3 dev/gen_split.py dev/split.txt 512");

    char* split_str;
    rdfile("dev/split.txt",&split_str);

    printf("cutils test\n");

    printf("Testing split\n");
    char **split_list = NULL;
    int count = splita(strdup(split_str),',',&split_list);

    // print list
    printf("split : printing list\n");
    char* str_split = assemble(split_list,count);
    free(*split_list);
    free(split_list);


    assert(strcmp(str_split,split_str) == 0);

    free(split_str);
    free(str_split);

    return;
}

void test_config() {
    assert(readConfig(getenv("SOVIET_CONFIG_FILE")) == 0);
    return;
}


void test_get() {

    char db_path[2048];
    sprintf(db_path,"%s/dev/get_test.db",CURRENT_DIR);

    setenv("ALL_DB_PATH",db_path,1);
    repo_sync();

    struct package base_pkg = {0};
    char base_path[2048];
    sprintf(base_path,"%s/dev/test.base.ecmp",CURRENT_DIR);
    assert(open_ecmp(base_path,&base_pkg) == 0);


    struct package t_pkg;
    t_pkg.name = "test";

    dbg(2,"Repo: %s",getenv("SOVIET_DEFAULT_REPO"));

    char out_test[2048+16 ] = {0};
    sprintf(out_test,"%s/dev/test.ecmp",CURRENT_DIR);
    dbg(3,"Copying to %s",out_test);
    remove(out_test);
    char* fmt = get(&t_pkg,getenv("SOVIET_DEFAULT_REPO"),out_test);
    
    assert(open_ecmp(out_test,&t_pkg) == 0);

    // print fmt and all package info
    dbg(1,"fmt: %s",fmt);
    dbg(1,"name: %s",t_pkg.name);
    dbg(1,"version: %s",t_pkg.version);
    dbg(1,"url: %s",t_pkg.url);

    assert(strcmp(base_pkg.name,t_pkg.name) == 0);
    assert(strcmp(base_pkg.version,t_pkg.version) == 0);
    assert(strcmp(base_pkg.url,t_pkg.url) == 0);
    // add other cmp later

    free_pkg(&base_pkg);
    free_pkg(&t_pkg);

    return;

}

void test_ecmp()
{
    setenv("FORMATS","ecmp",1);

    struct package old_pkg = {0};

    assert(open_ecmp(TEST_SPM_PATH,&old_pkg) == 0);
    
    // print the pkg
    printf("old_pkg: %s => %s %s\n",old_pkg.name,old_pkg.version,old_pkg.type);

    msg(INFO,"Creating ecmp package file");


    char mod_path[2048];
    sprintf(mod_path,"%s.mod",TEST_SPM_PATH);

    assert(create_ecmp(mod_path, &old_pkg) == 0);

    // now reopen package and compare
    struct package new_pkg = {0};

    assert(open_ecmp(mod_path,&new_pkg) == 0);

    // print the pkg
    dbg(2,"new_pkg: %s => %s %s\n",new_pkg.name,new_pkg.version,new_pkg.type);

    // compare packages
    assert(strcmp(new_pkg.name,old_pkg.name) == 0);
    assert(strcmp(new_pkg.version,old_pkg.version) == 0);
    assert(strcmp(new_pkg.type,old_pkg.type) == 0);

    // free packages
    free_pkg(&old_pkg);
    free_pkg(&new_pkg);

    return;
}

char* assemble(char** list,int count)
{
    dbg(3,"Assembling %d strings",count);
    char* string = calloc(2048*count,sizeof(char));
    int i;
    for (i = 0; i < count-1; i++)
    {
        strcat(string,list[i]);
        strcat(string,",");
        
    }
    strcat(string,list[i]);
    return string;
}
