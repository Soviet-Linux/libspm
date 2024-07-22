#include "stdio.h"
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

extern int open_spm(char* path,struct package* pkg);
extern int open_ecmp(char* path,struct package* pkg);

extern int create_spm(char* path,struct package* pkg);
extern int create_ecmp(char* path,struct package* pkg);

char* names[5] = {"pkg1","pkg2","pkg3","pkg4","pkg5"};
char* versions[5] = {"1.1.0","2.0.8","6.8.7","7.0","5.20"};
char* types[5] = {"bin","src","src","bin","src"};


char** list_of_stuff  = NULL;
int list_of_stuff_count = 0;

int test_ecmp();
int test_move();
int test_get();
int test_split();
int test_config();
int test_make(char* spm_path);

int test_install(char* spm_path);
int test_uninstall(char* pname);

char* assemble(char** list,int count);

char CURRENT_DIR[2048];

int main(int argc, char const *argv[])
{
    dbg(1, "started spm-test");
    if (argc  < 2)
    {
        printf("No arguments provided\n");
        return 1;
    }
    dbg(1, "Setting debug stuff");
    setenv("SOVIET_DEBUG","3",1);
    QUIET = false;
    OVERWRITE = true;
    DEBUG_UNIT = NULL;
    // we want to chnage that later 
    // TODO: Add hash to test package
    INSECURE = true;

    getcwd(CURRENT_DIR, 2048);

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
        int ret = 0;
        ret += test_ecmp();
        printf("Ret: %d\n",ret);
        ret += test_move();
        printf("Ret: %d\n",ret);
        ret += test_get();
        printf("Ret: %d\n",ret);
        ret += test_split();
        printf("Ret: %d\n",ret);
        ret += test_config();
        printf("Ret: %d\n",ret);
        ret += test_make("dev/vim.ecmp");
        printf("Ret: %d\n",ret);
        ret += test_install("dev/vim.ecmp");
        printf("Ret: %d\n",ret);
        ret += test_uninstall("vim");
        printf("Ret: %d\n",ret);
        int leaks = check_leaks();
        printf("Leaks: %d\n",leaks);
        ret += leaks; 
        return ret;
    } else if (strcmp(argv[1], "ecmp") == 0) {
        return test_ecmp();
    }  else if (strcmp(argv[1], "make") == 0) {
        int EXIT = 0;
        char* spm_path = NULL;
        if (argc < 3) {
            spm_path = strdup("dev/vim.ecmp");
        } else {
            spm_path = strdup(argv[2]);
        }
        EXIT += test_make(spm_path);

        free(spm_path);

        printf("Leaks: %d\n", check_leaks());
        return EXIT;
    }
    else if (strcmp(argv[1],"install") == 0)
    {
        char* spm_path = NULL;
        if (argc < 3)
        {
            spm_path = strdup("dev/vim.ecmp");
        } else {
            spm_path = strdup(argv[2]);
        }
        test_install(spm_path);

    } else if (strcmp(argv[1], "uninstall") == 0) {
        init();
        char* pname = NULL;
        if (argc > 3)
        { 
            pname = strdup(argv[2]);
        }
        else {
            pname = strdup("vim");
        }
        test_uninstall(pname);
        return 0;
    } else if (strcmp(argv[1], "move") == 0) {
        return test_move();
    } else if (strcmp(argv[1], "split") == 0) {
        return test_split();
    } else if (strcmp(argv[1], "config") == 0) {
        return test_config();
    } else if (strcmp(argv[1], "get") == 0) {
        return test_get();
    } else {
        printf("Invalid argument\n");
        return 1;
    }


}

int test_install(char* spm_path)
{
    dbg(1, "installing");
    init();
    install_package_source(spm_path, 0);
    printf("Leaks: %d\n", check_leaks());
    return 0;
}

int test_uninstall(char* pname)
{
    init();
    uninstall(pname);
    printf("Leaks: %d\n", check_leaks());
    return 0;
}

int test_move()
{
    
    init();

    rmrf("/tmp/spm-testing");

    #define l_d_count 5
    #define l_f_count 12
    #define l_l_count 3
    char* l_dirs[l_d_count] = {"b","b/d","s","s/j","s/j/k"};
    char* l_files[l_f_count] = {"w","b/d/e","a","d","b/y","b/c","b/f","s/j/k/z","s/j/k/x","s/j/k/c","s/j/k/v","s/j/k/b"};
    char* l_links[l_l_count][2] = {{"b/d/e","b/e"},{"s/j/k/z","s/z"},{"s/j/k/x","s/x"}};

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
    printf("Creating test links\n");
    // make all links
    for (int i = 0; i < l_l_count; i++)
    {
        printf("Creating %s -> %s\n",l_links[i][0],l_links[i][1]);
        char* old_path = malloc(256);
        sprintf(old_path,"%s/%s","/tmp/spm-testing/old",l_links[i][0]);
        char* new_path = malloc(256);
        sprintf(new_path,"%s/%s","/tmp/spm-testing/old",l_links[i][1]);
        symlink(old_path,new_path);
        free(old_path);
        free(new_path);
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


    move_binaries(end_locations,end_count);
    // Check if the move was successful
    int EXIT = 0;
    for (int i = 0; i < l_f_count; i++)
    {
        char* old_path = malloc(256);
        sprintf(old_path,"%s/%s","/tmp/spm-testing/old",l_files[i]);
        char* new_path = malloc(256);
        sprintf(new_path,"%s/%s","/tmp/spm-testing",l_files[i]);

        if(access(old_path, F_OK) != -1 || access(new_path, F_OK) == -1) {
            printf("Failed to move %s : \n",l_files[i]);
            printf("Old path: %s\n",old_path);
            printf("New path: %s\n",new_path);
            EXIT += 1;
            break;
        }

        free(old_path);
        free(new_path);
    }
    // check if the links were moved
    for (int i = 0; i < l_l_count; i++)
    {
        char* old_path = malloc(256);
        sprintf(old_path,"%s/%s","/tmp/spm-testing/old",l_links[i][0]);
        char* new_path = malloc(256);
        sprintf(new_path,"%s/%s","/tmp/spm-testing",l_links[i][1]);

        if(access(old_path, F_OK) != -1 || access(new_path, F_OK) == -1) {
            printf("Failed to move %s : \n",l_links[i][0]);
            printf("Old path: %s\n",old_path);
            printf("New path: %s\n",new_path);
            EXIT += 1;
            break;
        }

        free(old_path);
        free(new_path);
    }


    free(*end_locations);
    free(end_locations);


    printf("Testing move : done\n");

    printf("Leaks: %d\n",check_leaks());

    unsetenv("SOVIET_ROOT_DIR");
    unsetenv("SOVIET_BUILD_DIR");

    quit(0);
    return EXIT;
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
    chdir(CURRENT_DIR);
    system("python3 dev/gen_split.py dev/split.txt 512");

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

    // sync with remote
    repo_sync();

    struct package base_pkg = {0};
    char base_path[2048];
    sprintf(base_path,"%s/dev/test.base.ecmp",CURRENT_DIR);
    open_ecmp(base_path,&base_pkg);


    struct package t_pkg;
    t_pkg.name = "test";

    dbg(2,"Repo: %s",getenv("SOVIET_DEFAULT_REPO"));

    char out_test[2048+16 ] = {0};
    strcat(out_test, CURRENT_DIR);
    strcat(out_test, "/dev/test.ecmp");
    dbg(3,"Copying to %s",out_test);
    remove("dev/test.ecmp");
    char* fmt = get(&t_pkg,getenv("SOVIET_DEFAULT_REPO"),out_test);
    
    open_ecmp(out_test,&t_pkg);

    // print fmt and all package info
    printf("fmt: %s\n",fmt);
    printf("name: %s\n",t_pkg.name);
    printf("version: %s\n",t_pkg.version);
    printf("url: %s\n",t_pkg.url);

    dbg(3,"Comparing %s and %s",base_pkg.name,t_pkg.name);
    EXIT += strcmp(base_pkg.name,t_pkg.name);
    dbg(3,"Comparing %s and %s",base_pkg.version,t_pkg.version);
    EXIT += strcmp(base_pkg.version,t_pkg.version);
    dbg(3,"Comparing %s and %s",base_pkg.url,t_pkg.url);
    EXIT += strcmp(base_pkg.url,t_pkg.url);
    // add other cmp later

    free(fmt);
    free(base_pkg.name);
    free(base_pkg.version);
    free(base_pkg.url);
    free(t_pkg.name);
    free(t_pkg.version);
    free(t_pkg.url);

    printf("%d leaks\n",check_leaks());

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
