#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "job.h"
#include "arraylist.h"


void testJob(char* input){
    printf("Given |%s|, the\n", input);
    Job* job = makeJob(input);
    printJob(job);
    free(job);
}

/**
 * @brief Test the job.c makeJob() func here. No pipes.
 */
int main(int argc, char const *argv[])
{
    testJob("echo man name >    txt.txt");
    testJob("cat <  txt.txt     ");
    testJob("ls *.c");
    testJob("ls *.java >txt.txt");
    testJob("ls testFolder2/*.txt >txt.txt");
    testJob("pwd");
    return EXIT_SUCCESS;
}

