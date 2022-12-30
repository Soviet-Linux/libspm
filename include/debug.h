
enum level {
    INFO,
    ERROR,
    WARNING,
    FATAL
};

// a tool to have cool terminal output
int msg(enum level msgLevel, const char* message,...);
int f_dbg__(int level,int line,const char* function,const char* file,char* message,...);
#define dbg(level,message,...) f_dbg__(level,__LINE__,__func__,__FILE__,message,##__VA_ARGS__)