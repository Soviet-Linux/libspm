#include "test.h"

#include "../include/libspm.h"

char* assemble(char** list,int count);

extern int open_ecmp(char* path,struct package* pkg);
extern int create_ecmp(char* path,struct package* pkg);


void test_pm(char* spm_path) {

    msg(INFO,"Testing 'install_package_source()' and 'uninstall()'..");

    // install tests
    char temp_dir_template[] = "/tmp/test_dir_XXXXXX";
    char* test_dir = mkdtemp(temp_dir_template);
    setenv("SOVIET_ROOT",test_dir,1);
    char test_spm_dir[2048];
    sprintf(test_spm_dir,"%s/spm/",test_dir);
    setenv("SOVIET_SPM_DIR",test_spm_dir,1);

    init() ;

    assert(install_package_source(spm_path,0) == 0);
    assert(uninstall("vim") == 0);

    unsetenv("SOVIET_ROOT");    
    unsetenv("SOVIET_ROOT");

    return;
}




void test_move() {

    msg(INFO,"Testing 'move_binaries()'..");

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

    setenv("SOVIET_ROOT",test_dir,1);
    setenv("SOVIET_BUILD_DIR",build_dir,1);
    init();

    dbg(2,"creating test dirs");
    //make all dirs
    for (int i = 0; i < l_d_count; i++)
    {
        dbg(3,"Creating %s\n",l_dirs[i]);
        char* dir = malloc(256);
        sprintf(dir,"%s/%s",build_dir,l_dirs[i]);
        mkdir(dir,0777);
        free(dir);
    }
    dbg(2,"creating test files");
    // make all files
    for (int i = 0; i < l_f_count; i++)
    {        
        dbg(3,"Creating %s\n",l_files[i]);
        char* path = malloc(256);
        sprintf(path,"%s/%s",build_dir,l_files[i]);
        FILE* f = fopen(path,"w");
        fclose(f);
        free(path);
    }
    dbg(2,"Creating test links\n");
    // make all links
    for (int i = 0; i < l_l_count; i++)
    {
        dbg(3,"Creating %s -> %s\n",l_links[i][1],l_links[i][0]);
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

void test_make(char* spm_path) {

    // Set environment variables for building
    setenv("BUILD_ROOT", build_dir, 1);

    msg(INFO,"Testing 'make()'..");

    init();
    struct package p = {0};

    assert(open_pkg(spm_path, &p,NULL) == 0);

    setenv("NAME", p.name, 1);
    setenv("VERSION", p.version, 1);
    if (p.url != NULL) {
        parse_env(&(p.url));
        dbg(1, "URL: %s", p.url);
        setenv("URL", p.url, 1);
    }

    char* legacy_dir = calloc(2048,1);
    sprintf(legacy_dir,"%s/%s-%s",getenv("SOVIET_MAKE_DIR"),p.name,p.version);

    dbg(1,"Legacy dir: %s",legacy_dir);

    assert(make(legacy_dir,&p) == 0);

    // Run 'install' command
    if (p.info.install == NULL && strlen(p.info.install) == 0) {
        msg(FATAL, "No install command!");
    }

    char install_cmd[64 + strlen(legacy_dir) + strlen(p.info.install)];
    sprintf(install_cmd, "( cd %s && %s )", legacy_dir, p.info.install);

    dbg(2, "Executing install command: %s", install_cmd);
    if (system(install_cmd) != 0) {
        msg(FATAL, "Failed to install %s", p.name);
        return -2;
    }
    dbg(1, "Install command executed!");

    dbg(1,"Getting locations for %s",p.name);
    p.locationsCount = get_locations(&p.locations,getenv("SOVIET_BUILD_DIR"));
    assert(p.locationsCount > 0);
    
    free_pkg(&p);

    dbg(1,"Got %d locations for %s",p.locationsCount,p.name);

    return;
}

void test_split() {   

    msg(INFO,"Testing 'split()'..");

    chdir(WORKING_DIR);
    system("python3 gen_split.py split.txt 512");

    char* split_str;
    rdfile("split.txt",&split_str);

    char **split_list = NULL;
    int count = splita(strdup(split_str),',',&split_list);

    char* str_split = assemble(split_list,count);
    free(*split_list);
    free(split_list);


    assert(strcmp(str_split,split_str) == 0);

    free(split_str);
    free(str_split);

    return;
}

void test_config() {

    msg(INFO,"Testing 'readConfig()'..");

    assert(readConfig(getenv("SOVIET_CONFIG_FILE"), 0) == 0);
    return;
}


void test_get() {

    msg(INFO,"Testing 'get()'..");

    char db_path[2048];
    sprintf(db_path,"%s/get_test.db",WORKING_DIR);

    setenv("ALL_DB_PATH",db_path,1);
    repo_sync();

    struct package base_pkg = {0};
    char base_path[2048];
    sprintf(base_path,"%s/test.base.ecmp",WORKING_DIR);
    assert(open_ecmp(base_path,&base_pkg) == 0);


    struct package t_pkg;
    t_pkg.name = "test";

    dbg(2,"Repo: %s",getenv("SOVIET_DEFAULT_REPO"));

    char out_test[2048+16 ] = {0};
    sprintf(out_test,"%s/test.ecmp",WORKING_DIR);
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

void test_ecmp(char* spm_path) {

    msg(INFO,"Testing 'open_ecmp()' and 'create_ecmp()'..");

    setenv("FORMATS","ecmp",1);

    struct package old_pkg = {0};

    assert(open_ecmp(spm_path,&old_pkg) == 0);
    
    // print the pkg
    dbg(2,"old_pkg: %s => %s %s\n",old_pkg.name,old_pkg.version,old_pkg.type);

    msg(INFO,"Creating ecmp package file");


    char mod_path[2048];
    sprintf(mod_path,"%s.mod",spm_path);

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

    remove(mod_path);

    return;
}

char* assemble(char** list,int count) {
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
