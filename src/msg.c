#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "debug.h"
#include "globals.h"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

int msg(enum level msgLevel, const char* message,...) {
    va_list args;
    //initialize va_list args
    va_start(args, message);
    //declare size_t size
    size_t size = vsnprintf(NULL, 0, message, args);
    //allocate memory for strDest
    char strDest[size+2];
    //initialize va_list args
    va_start(args,message);
    //initialize vsnprintf
    vsnprintf(strDest, size+1, message, args);

    //initialize va_end
    va_end(args);


    switch (msgLevel)
    {
        case INFO:
            printf("%sINFO: %s%s%s%s\n",BOLDBLUE,RESET,BLUE,strDest,RESET);
            break;
        case WARNING:
            printf("%sWARNING: %s%s%s%s\n",BOLDYELLOW,RESET,YELLOW,strDest,RESET);
            break;
        case ERROR:
            printf("%sERROR: %s%s%s%s\n",BOLDMAGENTA,RESET,MAGENTA,strDest,RESET);
            break;
        case FATAL:
            printf("%sFATAL: %s%s%s%s\n",BOLDBLUE,RESET,BLUE,strDest,RESET);
            exit(1);
        default:
            printf("UNKNOWN: %s\n",strDest);
            break;
    }
    

    return 0;
}

// the debug function is bad
int f_dbg__(int level,int line,const char* function,const char* file,char* message,...) {

    file = strrchr(file,'/')+1;
    /* WARNING : EXPERIMENTAL */
    if (DEBUG_UNIT != NULL) {
        if (strcmp(DEBUG_UNIT,function) != 0) {
            return 1;
        }
    }

    va_list args;
    //initialize va_list args
    va_start(args, message);
    //declare size_t size
    size_t size = vsnprintf(NULL, 0, message, args);
    //allocate memory for strDest
    char strDest[size+2];
    //initialize va_list args
    va_start(args,message);
    //initialize vsnprintf
    vsnprintf(strDest, size+1, message, args);

    //initialize va_end
    va_end(args);

    if (DEBUG >= level)
    {
        printf("%s[%s:%d|%s()] %s%s%s%s\n",BOLDCYAN,file,line,function,RESET,GREEN,strDest,RESET);
    }
    return 0;
}