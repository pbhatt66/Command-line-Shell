#include <stddef.h>
#ifndef JOB
#define JOB
typedef struct Job {
    char* execPath;
    char** args;
    int numOfArgs;
    char* inputReDirectPath;
    char* outputReDirectPath;
} Job;

Job *makeJob(char* jobCmd);
void freeJob(Job* job);
void printJob(Job* job);
char* my_acess(char* pathToCheck, char* bareName, size_t lenOfBareName);

#define NUMOFPATHS 3
// char* path1 = "/usr/local/bin";
// char* path2 = "/usr/bin";
// char* path3 = "/bin";



extern char * paths[];
#endif

