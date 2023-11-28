#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "job.h"

/**
 * @brief Test the job.c makeJob() func here
 */
int main(int argc, char const *argv[])
{
    char* jobCmd = "foo < bar baz";
    Job* job = makeJob(jobCmd);
    printf("execPath: %s\n", job->execPath);
    printf("args: %s\n", job->args[0]);
    printf("inputReDirectPath: %s\n", job->inputReDirectPath);
    printf("outputReDirectPath: %s\n", job->outputReDirectPath);

    jobCmd = "foo bar > baz";
    Job* newjob = makeJob(jobCmd);
    printf("execPath: %s\n", newjob->execPath);
    printf("args: %s\n", newjob->args[0]);
    printf("inputReDirectPath: %s\n", newjob->inputReDirectPath);
    printf("outputReDirectPath: %s\n", newjob->outputReDirectPath);

    freeJob(job);
    freeJob(newjob);
    return 0;
}
