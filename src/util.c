#define _XOPEN_SOURCE 500
#include <ftw.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <dlfcn.h>

#include "math.h"
#include "libspm.h"
#include "cutils.h"
#include "globals.h"

// Callback function used by nftw to unlink files and directories
int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    (void)sb;
    (void)typeflag;
    (void)ftwbuf;

    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

// Recursively remove a directory and its contents
int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

// remove a file or link or directory
int rmany(char* path) 
{
    // check if its a symlink
    struct stat s;

    if (lstat(path, &s) == 0) {
        if (S_ISLNK(s.st_mode)) {
            // remove the symlink
             if (unlink(path) == 0) {
                return 0;
            } else {
                return -1;
            }
        }
        // check if its a directory
        if (S_ISDIR(s.st_mode)) {
            // remove the directory
            if (rmrf(path) == 0) {
                return 0;
            } else {
                msg(ERROR, "Error removing directory %s", path);
                return -1;
            }
        }
        // check if its a file
        if (S_ISREG(s.st_mode)) {
            // remove the file
            if (remove(path) == 0) {
                return 0;
            } else {
                return -1;
            }
        }
    } 
    return -1;
}

// Quit the program with the given status code and display an error message if status is not 0
void quit(int status) 
{
    if (status != 0) {
        msg(ERROR, "Exiting with status %d", status);
        exit(status);
    }
}

// Function to recursively retrieve all files in a directory and its subdirectories
char **get_all_files(const char* root, char *path, int *num_files) 
{
    DIR *dir;
    struct dirent *entry;
    struct stat stat_buf;
    char* origin = strdup(path);
    // Open the directory
    dir = opendir(path);
    if (dir == NULL) {
        // Print an error message if directory couldn't be opened
        fprintf(stderr, "Error opening directory %s: %s\n", path, strerror(errno));
        free(origin);
        return NULL;
    }

    // Initialize variables
    char **files_array = NULL;
    int file_count = 0;

    // Loop through directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip '.' and '..' entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Construct full path of the current entry
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        // Get information about the current entry
        if (stat(full_path, &stat_buf) == 0) {
            // If it's a directory, recursively call get_all_files
            if (S_ISDIR(stat_buf.st_mode)) {
                struct stat dir_stat_buf;
                if (lstat(full_path, &dir_stat_buf) == 0) {
                    // Check if a directory is a symlink (this can probably be optimized out)
                    if (!S_ISLNK(dir_stat_buf.st_mode)) {
                        // If it isn't - treat it as a directory
                        int sub_files_count;
                        char **sub_files = get_all_files(root, full_path, &sub_files_count);
                        if (sub_files != NULL) {
                            // Resize files_array and copy contents of sub_files into it
                            files_array = realloc(files_array, (file_count + sub_files_count) * sizeof(char *));
                            for (int i = 0; i < sub_files_count; i++) {
                                files_array[file_count++] = sub_files[i];
                            }
                            free(sub_files);
                        }
                    }
                    else {
                        // If it is - treat it as a file
                        files_array = realloc(files_array, (file_count + 1) * sizeof(char *));
                        files_array[file_count] = strdup(full_path + strlen(root) + 1);
                        file_count++;
                    }
                }
            } else if (S_ISREG(stat_buf.st_mode)) {
                // If it's a regular file, add it to files_array
                files_array = realloc(files_array, (file_count + 1) * sizeof(char *));
                files_array[file_count] = strdup(full_path + strlen(root) + 1);
                file_count++;
            }
        }
    }
    // Close the directory
    closedir(dir);

    // Update num_files if it's not NULL
    if (num_files != NULL)
        *num_files = file_count;

    // Return the array of file paths
    free(origin);

    return files_array;
}

// Load a format plugin, execute a specific function, and close the plugin
int runFormatLib(const char* format, const char* fn, const char* pkg_path, struct package* pkg) 
{
    char lib_path[MAX_PATH];
    sprintf(lib_path, "%s/%s.so", getenv("SOVIET_PLUGIN_DIR"), format);
    dbg(2, "Loading %s", lib_path);

    if (access(lib_path, F_OK) != 0) {
        msg(ERROR, "File %s does not exist", lib_path);
        return 1;
    }

    // Load a function from the shared library
    void* handle = dlopen(lib_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        return 1;
    }
    int (*func)(const char*, struct package*) = dlsym(handle, fn);
    char* error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "%s\n", error);
        return 1;
    }
    if (func(pkg_path, pkg) != 0) {
        return -1;
    }

    dlclose(handle);
    return 0;
}

// A function to retrieve the version number of the libspm library.
float version()
{
    return LIBSPM_VERSION;
}

// This will parse a string for environment variables
// It makes an assumption that a variable is: $A-Z_0-9
int parse_env(char** in)
{
    dbg(2, "Parsing string %s for env variables", *in);
    char* env = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_";
    char* start = strchr(*in, '$');
    char* end = NULL;
    size_t i, start_i;

    if (start == NULL)
    {
        return 0;
    }

    start_i = strlen(*in) - strlen(start);    
    for (i = 1; i < strlen(start); i++)
    {
        end = strchr(env, start[i]);

        if (end == NULL)
        {
            if(i == 0)
            {
                return 0;
            }

            if(start[i] != '\0')
            {
                end = &start[i];
            }

            break;
        }

        if(i + 1 == strlen(start))
        {
            end = "";
        }
    }

    char* var = strdup(*in + start_i + 1);
    char* dup_in = calloc(start_i + 1, 1);
    if(start_i != 0)
    {
        snprintf(dup_in, start_i + 1, "%s", *in);
    }
    var[--i] = '\0';

    dbg(2, "Found variable: %s", var);

    char* full_var = getenv(var);

    if(full_var == NULL)
    {
        return 0;
    }

    dbg(2, "Substituting for: %s", full_var);

    char* full_in = calloc(strlen(*in) + strlen(full_var) + strlen(end) + 1, 1);

    sprintf(full_in, "%s%s%s", dup_in, full_var, end);
    
    free(*in);
    free(dup_in);
    free(var);
    *in = full_in;

    dbg(2, "Result: %s", *in);

    return parse_env(in);
}

// Download a file from url into FILE 
int download(char* url, FILE* fp) {
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
	    (void) res;
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "CCCP/1.0 (https://www.sovietlinux.org/)(rip i guess)");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return 0;
}

// Copy a file
int cp(char* from, char* to)
{
    struct stat st;
    stat(from, &st);
    int size = st.st_size;
    int permissions = st.st_mode;
    int owner = st.st_uid;
    int group = st.st_gid;

    char* buffer = malloc(size);

    FILE *old_ptr;
    FILE *new_ptr;

    old_ptr = fopen(from,"r"); 
    if (old_ptr == NULL) {
        msg(ERROR,"Error opening file %s",from);
        return -1;
    }
    fread(buffer, sizeof(char), size, old_ptr); 
    fclose(old_ptr);

    new_ptr = fopen(to,"w"); 
    if (new_ptr == NULL) {
        free(buffer);
        msg(ERROR,"Error opening file %s",to);
        return -2;
    }
    fwrite(buffer, sizeof(char), size, new_ptr);
    int result = fclose(new_ptr);

    if (result != 0) {
        free(buffer);
        msg(ERROR,"Error writing to file %s",to);
        return -3;
    }

    free(buffer);

    if (chown(to, owner, group) != 0) {
        msg(ERROR,"Error changing owner of %s",to);
        return -4;
    }
    if (chmod(to, permissions) != 0) {
        msg(ERROR,"Error changing permissions of %s",to);
        return -5;
    }

    return 0;
}

// Ask a yes/no queston
int get_input(char* prompt, int def)
{
    char* str = calloc(2, sizeof(char));
    char def_char;
    
    if(def == 0)
    {
        def_char = 'N';
        printf("%s (y/N)\n", prompt);
    }
    else 
    {
        def_char = 'Y';
        printf("%s (Y/n)\n", prompt);
    }

    
    if(AUTO)
    {
        free(str);
        return def;
    }
    else
    {
        fgets(str, 2, stdin);
        if ( strchr(str, '\n') == NULL )
        {
            while ((getchar()) != '\n');
        }

        int i = 0;

        while (str[i] != '\n' && str[i] != '\0')
        {
            i++;
        }

        if (str[i] == '\n')
        {
            str[i] = '\0';
        }
    }
    if(str[0] == '\0')
    {
        str[0] = def_char;
        str[1] = '\0';
    }
    if(str[0] == 'Y' || str[0] == 'y')
    {
        free(str);
        return 1;
    }
    free(str);
    return 0;
}