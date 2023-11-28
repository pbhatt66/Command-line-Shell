#include "arraylist.h"
typedef struct Job {
    char* execPath;
    char** args;
    char* inputReDirectPath;
    char* outputReDirectPath;
} Job;

Job *makeJob(char* jobCmd);
void freeJob(Job* job);
