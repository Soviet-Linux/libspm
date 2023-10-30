#include "stdio.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

#include "../include/globals.h"
#include "../include/libspm.h"
#include "../include/data.h"
#include "../include/cutils.h"
#include "../include/libspm.h"


#define STATIC

extern int open_spm(char* path,struct package* pkg);
extern int open_ecmp(char* path,struct package* pkg);

extern int create_spm(char* path,struct package* pkg);
extern int create_ecmp(char* path,struct package* pkg);

char* names[5] = {"pkg1","pkg2","pkg3","pkg4","pkg5"};
char* versions[5] = {"1.1.0","2.0.8","6.8.7","7.0","5.20"};
char* types[5] = {"bin","src","src","bin","src"};

#define l_d_count 5
#define l_f_count 12
char* l_dirs[l_d_count] = {"b","b/d","s","s/j","s/j/k"};
char* l_files[l_f_count] = {"w","b/d/e","a","d","b/y","b/c","b/f","s/j/k/z","s/j/k/x","s/j/k/c","s/j/k/v","s/j/k/b"};

char** list_of_stuff  = NULL;
int list_of_stuff_count = 0;

int test_data ();
int test_ecmp();
int test_move();
int test_get();
int test_split();
int test_config();
int test_make(char* spm_path);

char* assemble(char** list,int count);

int main(int argc, char const *argv[])
{

    if (argc  < 2)
    {
        printf("No arguments provided\n");
        return 1;
    }
    printf("No not here\n");
    DEBUG = 4;
    printf("Nor here\n");
    QUIET = false;
    printf("and No not here\n");
    OVERWRITE = true;
    printf("No not even here\n");
    DEBUG_UNIT = NULL;

    

    printf("wtf\n");
    if (strcmp(argv[1],"data") == 0)
    {
        return test_data();
    }
    else if (strcmp(argv[1],"ecmp") == 0)
    {
        return test_ecmp();
    }
    else if (strcmp(argv[1],"all") == 0)
    {
        int ret = 0;
        ret += test_data();
        ret += test_ecmp();
        return ret;
    }
    else if (strcmp(argv[1],"make") == 0)
    {
        int EXIT = 0;
        EXIT += test_make(argv[2]);
        printf("Leaks: %d",check_leaks());
        return EXIT;
    }
    else if (strcmp(argv[1],"install") == 0)
    {
        init();
        install_package_source(argv[2],0);
        printf("Leaks: %d\n",check_leaks());
        return 0;
    }
    else if (strcmp(argv[1],"uninstall") == 0)
    {
        init();
        uninstall(argv[2]);
        return 0;
    }
    else if (strcmp(argv[1],"move") == 0)
    {
        return test_move();
    }
    else if (strcmp(argv[1],"help") == 0)
    {
        printf("Usage: test [spm|data|ecmp|all|help|install]\n");
        return 0;
    }
    else if (strcmp(argv[1],"split") == 0)
    {
        return test_split();
    }

    else if (strcmp(argv[1],"config") == 0)
    {
        return test_config();
    }
    else if (strcmp(argv[1],"get") == 0)
    {
        return test_get();
    }
    else
    {
        printf("Invalid argument\n");
        return 1;
    }


}

int test_move()
{
    
    init();
    printf("Testing move\n");
    setenv("SOVIET_ROOT","/tmp/spm-testing",1);
    setenv("SOVIET_BUILD_DIR","/tmp/spm-testing/old",1);
    printf("Creating directories\n");
    rmrf("/tmp/spm-testing");
    printf("Creating directories\n");
    mkdir("/tmp/spm-testing",0777);
    printf("Creating directories\n");
    mkdir("/tmp/spm-testing/old",0777);
    printf("Creating directories\n");

    printf("Creating test dirs\n");
    //make all dirs
    for (int i = 0; i < l_d_count; i++)
    {
        printf("Creating %s\n",l_dirs[i]);
        char* dir = malloc(256);
        sprintf(dir,"%s/%s","/tmp/spm-testing/old",l_dirs[i]);
        mkdir(dir,0777);
        free(dir);
    }
    printf("creating test files\n");
    // make all files
    for (int i = 0; i < l_f_count; i++)
    {
        printf("Creating %s\n",l_files[i]);
        char* path = malloc(256);
        sprintf(path,"%s/%s","/tmp/spm-testing/old",l_files[i]);
        printf("Path: %s\n",path);
        FILE* f = fopen(path,"w");
        fclose(f);
        free(path);
    }

    // get all the files with get_locatins
    char** end_locations;
    int end_count = get_locations(&end_locations,"/tmp/spm-testing/old");

    // print end locations
    printf("End locations:\n");
    for (int i = 0; i < end_count; i++)
    {
        printf("%s\n",end_locations[i]);
    }


    move_binaries(end_locations,8);
    free(*end_locations);
    free(end_locations);

    printf("Testing move : done\n");

    printf("Leaks: %d\n",check_leaks());

    quit(0);
    return 0;
}

int test_make(char* spm_path) {

    init();

    struct package p = {0};

    open_pkg(spm_path, &p,NULL);

    printf("Testing make\n");

    char* legacy_dir = calloc(2048,1);
    sprintf(legacy_dir,"%s/%s-%s",getenv("SOVIET_MAKE_DIR"),p.name,p.version);

    dbg(1,"Legacy dir: %s",legacy_dir);

    make(legacy_dir,&p);

    dbg(1,"Getting locations for %s",p.name);
    p.locationsCount = get_locations(&p.locations,getenv("SOVIET_BUILD_DIR"));
    if (p.locationsCount <= 0) {
        msg(ERROR,"Failed to get locations for %s",p.name);
        return -1;
    }
    dbg(1,"Got %d locations for %s",p.locationsCount,p.name);

    return 0;
}

int test_split()
{
    char* split_str;
    rdfile("dev/split.txt",&split_str);

    printf("cutils test\n");

    printf("Testing split\n");
    char **split_list = NULL;
    int count = splita(strdup(split_str),',',&split_list);
    printf("split : %d\n",count);
    printf("list[0] : %s\n",split_list[0]);
    printf("list[1] : %s\n",split_list[1]);

        // print list
    printf("split : printing list\n");
    char* str_split = assemble(split_list,count);
    free(*split_list);
    free(split_list);


    if (strcmp(str_split,split_str) != 0)
    {
        printf("split : failed\n");
        printf("splitted/assembled string : %s\n",str_split);
        printf("OG string : %s\n",split_str);
        return 1;
    }
    else
    {
        printf("split : passed\n");
    }

    free(split_str);
    free(str_split);

    printf("%d Leaks\n",check_leaks());
    return 0;
}

int test_config()
{
    int EXIT = EXIT_SUCCESS;
    printf("cutils test\n");

    printf("Testing config\n");
    EXIT += readConfig(getenv("SOVIET_CONFIG_FILE"));

    printf("%d Leaks\n",check_leaks());
    return EXIT;
}


int test_get()
{
    dbg(2,"Getting 'test' package\n");

    setenv("ALL_DB_PATH","dev/get_test.db",1);


    init();
    int EXIT = 0;

    

    struct package t_pkg;
    t_pkg.name = "test";


    char* fmt = get(&t_pkg,"dev/test");
    
    // print fmt and all package info
    printf("fmt: %s\n",fmt);
    printf("name: %s\n",t_pkg.name);
    printf("version: %s\n",t_pkg.version);
    printf("type: %s\n",t_pkg.type);

    free(fmt);

    return 0;

}
int test_data ()
{
    //init();
    int EXIT = EXIT_SUCCESS;

    setenv("ALL_DB_PATH","dev/all.db",1);

    printf("Removing test.db\n");
    remove("dev/all.db");

    dbg(2,"Creating test database...\n");

    sqlite3 *db;
    connect_db(&db,"dev/all.db");
    create_table_installed(db);

    
    dbg(2,"Adding data to test database...\n");
    for (int i = 0; i < 5; i++) {
        struct package a_pkg = {0};
        a_pkg.name = names[i];
        a_pkg.version = versions[i];
        a_pkg.type = types[i];
        store_data_installed(db,&a_pkg,0);
    }
    
    dbg(2,"Print all data from test database...\n");
    print_all_data(db);
    dbg(2,"Checking data in test database...\n");
    for (int i = 0; i < 5; i++)
    {
        struct package a_pkg = {0};
        a_pkg.name = names[i];
        printf("Checking %s...\n",a_pkg.name);
        retrieve_data_installed(db,&a_pkg,NULL);
        msg(INFO,"  %s => %s %s\n",a_pkg.name,a_pkg.version,a_pkg.type);
        if (strcmp(a_pkg.version,versions[i]) != 0 |
            strcmp(a_pkg.type,types[i]) !=0 ) {
            msg(ERROR,"Invalid return values , database check failed");
            EXIT = -1;
        }
        free(a_pkg.version);
        free(a_pkg.type);
        
    }
    printf("Removing data from test database...\n");
    for (int i = 0; i < 5; i++) {
        printf("Removing %s\n",names[i]);
        remove_data_installed(db,names[i]);
    }
    

    printf("Checking data in test database...\n");
    for (int i = 0; i < 5; i++) {
        struct package a_pkg;
        a_pkg.name = names[i];
        a_pkg.version = NULL;
        a_pkg.type = NULL;
        retrieve_data_installed(db,&a_pkg,NULL);

        printf("  %s => %s %s\n",a_pkg.name,a_pkg.version,a_pkg.type);
        if (a_pkg.type != NULL | a_pkg.version != NULL)
        {
            msg(ERROR,"Database supression failed ");
            EXIT = -1;
        }

    }

    printf("Closing test database...\n");
    sqlite3_close(db);
    printf("Removing test database...\n");
    remove("dev/all.db");



    printf("%d Leaks\n",check_leaks());

    return EXIT;
}



int test_ecmp(int type)
{
    setenv("FORMATS","ecmp",1);
    int EXIT = EXIT_SUCCESS;

    struct package old_pkg = {0};

    EXIT += open_ecmp("dev/vim.ecmp",&old_pkg);
    
    // print the pkg
    printf("old_pkg: %s => %s %s\n",old_pkg.name,old_pkg.version,old_pkg.type);

    msg(INFO,"Creating ecmp package file");

    EXIT += create_ecmp("dev/vim.mod.ecmp", &old_pkg);

    // now reopen package and compare
    struct package new_pkg = {0};

    EXIT += open_ecmp("dev/vim.mod.ecmp",&new_pkg);

    // print the pkg
    printf("new_pkg: %s => %s %s\n",new_pkg.name,new_pkg.version,new_pkg.type);

    // compare packages
    EXIT += new_pkg.name ? strcmp(new_pkg.name,old_pkg.name) : 1;
    EXIT += new_pkg.version ? strcmp(new_pkg.version,old_pkg.version) : 1;
    EXIT += new_pkg.type ? strcmp(new_pkg.type,old_pkg.type) : 1;

    dbg(3,"Exiting with %d",EXIT);

    // free packages
    free(old_pkg.name);
    free(old_pkg.version);
    free(old_pkg.type);
    free(old_pkg.url);
    free(old_pkg.info.download);
    free(old_pkg.info.install);
    free(old_pkg.info.special);
    free(old_pkg.license);
    
    free(new_pkg.name);
    free(new_pkg.version);
    free(new_pkg.type);
    free(new_pkg.url);
    free(new_pkg.info.download);
    free(new_pkg.info.install);
    free(new_pkg.info.special);
    free(new_pkg.license);

    printf("%d leaks\n",check_leaks());

    return EXIT;
}

char* assemble(char** list,int count)
{
    dbg(3,"Assembling %d strings",count);
    char* string = calloc(32*count,sizeof(char));
    int i;
    for (i = 0; i < count-1; i++)
    {
        strcat(string,list[i]);
        strcat(string,",");
        
    }
    strcat(string,list[i]);
    return string;
}
