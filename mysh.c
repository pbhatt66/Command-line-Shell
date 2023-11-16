#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "job.h"
#ifndef DEBUG
    #define DEBUG 1
#endif
#define BUFSIZE 256
#define MYSH_EXIT_SUCCESS 1
#define MYSH_EXIT_FAILURE 0
#define COND_END_INDEX 4

/**
 * @brief This is the main utilty function. It takes in a cmd line
 * and then parse through it. Calls build job to make the jobs if pipes are included.
 * Once the array of jobs are built, it will call loop through each job, and deal with
 * making pipes, input/output redirects, and conditionals.
 * 
 * @param cmd 1 cmd literal
 * @return int MYSH_EXIT_SUCCESS | MYSH_EXIT_FAILURE
 */
int accept_cmd_line(char *cmd) {
    if (strcmp("exit", cmd) == 0) {
        printf("mysh: exiting\n");
        exit(EXIT_SUCCESS);
    }
    printf("Got a line: |%s|\n", cmd);
    int cmdlen = strlen(cmd);

    int currentStartOfJob = 0, jobIndex = 0;

    //Job** jobList = malloc(sizeof(Job *) * 2);
    char** jobList = malloc(sizeof(char *) * 2);

    for(int i = 0; i < cmdlen + 1; i++){
        char currentChar = cmd[i];
        if(currentChar == '|' || currentChar == '\0'){
            int lenOfJob = i - currentStartOfJob;
            char* job = malloc(lenOfJob + 1);
            memcpy(job, cmd + currentStartOfJob, lenOfJob);
            job[lenOfJob] = '\0';
            //jobList[jobIndex] = makeJob(job);
            jobList[jobIndex] = job;
            currentStartOfJob = i+1;
            jobIndex++;
        }
    }

    int numOfJobs = jobIndex;
    jobIndex = 0;
    
    


    printf("Len of Job 1: %lu\n", strlen(jobList[0]));
    printf("Job 1: |%s|\n", jobList[0]);
    if(jobIndex > 1){
        printf("Len of Job 2: %lu\n", strlen(jobList[1]));
        printf("Job 2: |%s|\n", jobList[1]);
    }
    else{
        //no pipe
    }


    // //for(int i)
    return MYSH_EXIT_SUCCESS;
}
/**
 * @brief Prints "mysh> ", and force flushes stdout, so the printf call 
 * is printed since there is no "\n" character
 */
void promptNextCMD(){
    printf("mysh> ");
    fflush(stdout);
}
/**
 * @brief This main method reads from STDIN_FILENO or bash file line by line
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv) {
    printf("Welcome to my shell!\n");

    char buf[BUFSIZE];
    int pos, bytes, line_length = 0, fd, start;
    char *line = NULL;
    int SHOWPROMPTS = 0, mysh_errno = MYSH_EXIT_SUCCESS;

    if (argc > 1) {
        fd = open(argv[1], O_RDONLY);
        SHOWPROMPTS = 0;
        if (fd < 0) {
            perror(argv[1]);
            exit(EXIT_FAILURE);
        }
    } else {
        fd = STDIN_FILENO;
        SHOWPROMPTS = 1;
    }
    if (SHOWPROMPTS) promptNextCMD();
    while ((bytes = read(fd, buf, BUFSIZE)) > 0) {
        if (DEBUG) printf("Read %d bytes\n", bytes);
        // search input for a newline character
        start = 0;
        for (pos = 0; pos < bytes; pos++) {
            if (buf[pos] == '\n') {
                int len = pos - start;
                line = realloc(line, line_length + len + 1);
                memcpy(line + line_length, buf + start, len);
                line_length = line_length + len;
                line[line_length] = '\0';
                if(strncmp(line, "then", COND_END_INDEX) == 0){
                    printf("Checking then\n");
                    if(mysh_errno == MYSH_EXIT_SUCCESS){
                        mysh_errno = accept_cmd_line(line+COND_END_INDEX+1);
                    }
                    else {
                        //used "then", but prev cmd retured a error
                        //so the new cmd is not run
                    }
                        
                }
                else if(strncmp(line, "else", COND_END_INDEX) == 0){
                    printf("Checking else\n");
                    if(mysh_errno == MYSH_EXIT_FAILURE){
                        mysh_errno = accept_cmd_line(line+COND_END_INDEX+1);
                    }
                    else {
                        //used "else", but prev cmd did not return an error
                        //so the new cmd is not run
                    }      
                }
                else{
                    //no conditionals needs to be checked
                    mysh_errno = accept_cmd_line(line);
                }
                free(line);
                line = NULL;
                line_length = 0;
                start = pos + 1;
            }
        }
        if (DEBUG) printf("End of buffer; start=%d pos=%d\n", start, pos);

        // check for a partial line
        if (start < bytes) {
            int len = bytes - start;
            line = realloc(line, line_length + len + 1);
            memcpy(line + line_length, buf + start, len);
            line_length += len;
            line[line_length] = '\0';
            if (DEBUG)
                printf("Partial line |%s|; %d bytes\n", line, line_length);
        }
        if (SHOWPROMPTS) promptNextCMD();

    }

    printf("mysh: exiting\n");
    close(fd);
    return EXIT_SUCCESS;
}
