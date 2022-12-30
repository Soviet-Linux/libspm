#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "malloc.h"

#include "debug.h"

int log_ptr(void* ptr,char* file,int line);
int unlog_ptr(void* ptr,char* file,int line);
char* find_dfree(char* ptr);

void* dbg_malloc(size_t size,char* file,int line)
{
    dbg(4,"dbg_malloc : %s:%d  - %zu bytes",file,line,size);
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
        printf("dbg_malloc : malloc failed");
    }
    log_ptr(ptr,file,line);

    return ptr;
}

void* dbg_calloc(size_t nmemb,size_t size,char* file,int line)
{
    dbg(4,"dbg_calloc : %s:%d - %d bytes",file,line,nmemb*size);
    void* ptr = calloc(nmemb,size);
    if (ptr == NULL)
    {
        msg(FATAL,"dbg_calloc : calloc failed");
    }
    log_ptr(ptr,file,line);
    return ptr;
}

void* dbg_realloc(void* ptr,size_t size,char* file,int line)
{
    dbg(4,"dbg_realloc : %s:%d %p %d->%d bytes",file,line,ptr,malloc_usable_size(ptr),size);
    void* newptr = realloc(ptr,size);
    if (newptr == NULL)
    {
        msg(FATAL,"dbg_realloc : realloc failed");
    }
    unlog_ptr(ptr,file,line);
    log_ptr(newptr,file,line);
    return newptr;
}

char* dbg_strdup(char* str,char* file,int line)
{
    dbg(4,"dbg_strdup : %s:%d  - %d bytes",file,line,strlen(str)+1);
    char* newstr = strdup(str);
    if (newstr == NULL)
    {
        msg(FATAL,"dbg_strdup : strdup failed");
    }
    log_ptr(newstr,file,line);
    return newstr;
}

void dbg_free(void* ptr,char* file,int line)
{
    dbg(4,"dbg_free : %s:%d %p  - %d bytes",file,line,ptr,malloc_usable_size(ptr));
    if (unlog_ptr(ptr,file,line) != 0)
    {
        char* pos = find_dfree(ptr);
        if (pos != NULL)
        {
            msg(FATAL,"dbg_free : trying to free already freed at %s pointer --> %p",pos,ptr);
        }
        msg(FATAL,"dbg_free : trying to free unallocated pointer --> %p",ptr);
    }
    free(ptr);
    
}


char alloced_ptrs[4096][2][64] = {0};
int alloced_ptrs_i = 0;

char freed_ptrs[4096][2][64] = {0};
int freed_ptrs_i = 0;


int log_ptr(void* ptr,char* file,int line)
{

    char* hex = alloced_ptrs[alloced_ptrs_i][0];
    sprintf(hex,"%p",ptr);
    dbg(4,"Logging ptr %s",hex);

    // concatenate the info
    char* info = alloced_ptrs[alloced_ptrs_i][1];
    sprintf(info,"%s:%d",file,line);

    alloced_ptrs_i++;
    return 0;
}

int unlog_ptr(void* ptr,char* file,int line)
{
    char ptr_hex[64];
    sprintf(ptr_hex,"%p",ptr);

    for (int i = 0; i < alloced_ptrs_i; i++)
    {
        char* hex = alloced_ptrs[i][0];
        if (strcmp(hex,ptr_hex) == 0)
        {   
            // add to freed_ptrs
            char* freed_hex = freed_ptrs[freed_ptrs_i][0];
            char* freed_info = freed_ptrs[freed_ptrs_i][1];        
            strcpy(freed_hex,hex);

            sprintf(freed_info,"%s:%d",file,line);
            freed_ptrs_i++;

            alloced_ptrs[i][0][0] = 0;
            alloced_ptrs[i][1][0] = 0;
            return 0;
        }
    }
    return 1;
}

char* find_dfree(char* ptr)
{
    char ptr_hex[64];
    sprintf(ptr_hex,"%p",ptr);

    for (int i = 0; i < freed_ptrs_i; i++)
    {
        char* hex = freed_ptrs[i][0];
        if (strcmp(hex,ptr_hex) == 0)
        {
            return freed_ptrs[i][1];
        }
    }
    return NULL;
}

int dbg_leaks()
{
    int leaks = 0;
    for (int i = 0; i < alloced_ptrs_i; i++)
    {
        char* hex = alloced_ptrs[i][0];
        if (hex[0] != 0)
        {
            leaks++;
            printf("Leak : %s : %s \n",hex,alloced_ptrs[i][1]);
        }
    }
    return leaks;
}