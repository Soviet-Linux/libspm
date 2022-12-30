#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void* dbg_malloc(size_t size,char* file,int line);
void* dbg_calloc(size_t nmemb,size_t size,char* file,int line);
void* dbg_realloc(void* ptr,size_t size,char* file,int line);
char* dbg_strdup(char* str,char* file,int line);

void dbg_free(void* ptr,char* file,int line);

//#define MEMCHECK

#ifdef MEMCHECK // only define these if we're doing memory checking
    #define malloc(size) dbg_malloc(size,__FILE__,__LINE__)
    #define calloc(nmemb,size) dbg_calloc(nmemb,size,__FILE__,__LINE__)
    #define realloc(ptr,size) dbg_realloc(ptr,size,__FILE__,__LINE__)
    #define strdup(str) dbg_strdup(str,__FILE__,__LINE__)
    #define free(ptr) dbg_free(ptr,__FILE__,__LINE__)
#endif

int check_leaks();