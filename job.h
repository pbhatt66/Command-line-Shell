#include "arraylist.h"
typedef struct Job {
    char* execPath;
    arraylist_t args;
    char* inputReDirectPath;
    char* outputReDirectPath;
} Job;

Job *makeJob(char* jobCmd);
