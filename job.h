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