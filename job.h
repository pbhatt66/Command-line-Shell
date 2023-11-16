typedef struct Job {
    char* execPath;
    char** args; //change to arraylist from the code on canvas
    char* inputReDirectPath;
    char* outputReDirectPath;
} Job;

Job *makeJob(char* jobCmd);
