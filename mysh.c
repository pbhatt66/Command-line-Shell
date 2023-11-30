#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include "job.h"
#ifndef DEBUG
    #define DEBUG 1
#endif
#define BUFSIZE 256
#define MYSH_EXIT_SUCCESS 1
#define MYSH_EXIT_FAILURE 0
#define COND_END_INDEX 4

// int runJob(Job* job, int pipeId){
//     // run 1 job
//     if (fork() == 0) {
//         if(strcmp(job->inputReDirectPath, "") != 0){
//             //input redircetion
//             int fd = open(job->inputReDirectPath, O_RDONLY);
//             if(fd != -1){
//                 dup2(fd, STDIN_FILENO);
//             } else{
//                 perror(job->args[0]);
//             }
//         }
//         if(strcmp(job->outputReDirectPath, "") != 0){
//             //output redircetion
//             int fd = open(job->outputReDirectPath, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
//             if(fd != -1){
//                 dup2(fd, STDOUT_FILENO);
//             } else{
//                 perror(job->args[0]);
//             }
//         }
//         execv(job->execPath, job->args);
//         perror("ERROR\n");
//     }
//     int child_status;
//     wait(&child_status);
//     if(child_status == 0)
//         return MYSH_EXIT_SUCCESS;
//     else   
//         return MYSH_EXIT_FAILURE;

//     // // opening two processes and establishing a pipe
//     // int fd[2];
//     // pipe(fd);
//     // if (fork() == 0) {
//     //     // first child
//     //     dup2(fd[1], STDOUT_FILENO);
//     //     execl(first_program, ..., NULL);
//     //     exit(1); //error
//     // }
//     // close(fd[1]);
//     // if (fork() == 0) {
//     //     // second child
//     //     dup2(fd[0], STDIN_FILENO);
//     //     execl(second_program, ..., NULL);
//     //     exit(1); //error
//     // }
// }

int runJob(Job* job){
    if(strcmp(job->inputReDirectPath, "") != 0){
        //input redircetion
        int fd = open(job->inputReDirectPath, O_RDONLY);
        if(fd != -1){
            dup2(fd, STDIN_FILENO);
        } else{
            fprintf(stderr, "mysh: %s: %s\n", strerror(errno),job->inputReDirectPath);
            exit(EXIT_FAILURE);
            return MYSH_EXIT_FAILURE;
        }
    }
    if(strcmp(job->outputReDirectPath, "") != 0){
        //output redircetion
        int fd = open(job->outputReDirectPath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
        if(fd != -1){
            dup2(fd, STDOUT_FILENO);
        } else{
            fprintf(stderr, "mysh: %s: %s\n", strerror(errno),job->outputReDirectPath);
            exit(EXIT_FAILURE);
            return MYSH_EXIT_FAILURE;
        }
    }
    execv(job->execPath, job->args);
    fprintf(stderr, "mysh: command not found: %s\n", job->args[0]);
    exit(EXIT_FAILURE);

    // // opening two processes and establishing a pipe
    // int fd[2];
    // pipe(fd);
    // if (fork() == 0) {
    //     // first child
    //     dup2(fd[1], STDOUT_FILENO);
    //     execl(first_program, ..., NULL);
    //     exit(1); //error
    // }
    // close(fd[1]);
    // if (fork() == 0) {
    //     // second child
    //     dup2(fd[0], STDIN_FILENO);
    //     execl(second_program, ..., NULL);
    //     exit(1); //error
    // }
}

int runJobs(Job** jobs, int numOfJobs){
    if(numOfJobs == 1){
        if (fork() == 0) {
            runJob(jobs[0]);
        }
        int child_status;
        wait(&child_status);
        if(child_status == 0)
            return MYSH_EXIT_SUCCESS;
        else   
            return MYSH_EXIT_FAILURE;
    }
    else{
        //pipes
        int fd[2];
        pipe(fd);
        if (fork() == 0) {
            //first child
            dup2(fd[1], STDOUT_FILENO);
            runJob(jobs[0]);
        }
        int child_status;
        wait(&child_status);
        if(child_status != 0)
            return MYSH_EXIT_FAILURE;
        close(fd[1]);
        if (fork() == 0) {
            //second child
            dup2(fd[0], STDIN_FILENO);
            runJob(jobs[1]);
        }
        int child_status2;
        wait(&child_status2);
        if(child_status2 != 0)
            return MYSH_EXIT_FAILURE;
        else
            return EXIT_SUCCESS;
    }
}

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
    //printf("Got a line: |%s|\n", cmd);
    // int cmdlen = strlen(cmd);

    // int currentStartOfJob = 0, numOfJobs = 0;

    // Job** jobList = malloc(sizeof(Job *) * 2);
    // jobList[0] = NULL;
    // jobList[1] = NULL;
    //char** jobList = malloc(sizeof(char *) * 2);

    // for(int i = 0; i < cmdlen + 1 && numOfJobs < 2; i++){
    //     char currentChar = cmd[i];
    //     if(currentChar == '|' || currentChar == '\0'){
    //         int lenOfJob = i - currentStartOfJob;
    //         char* job = malloc(lenOfJob + 1);
    //         memcpy(job, cmd + currentStartOfJob, lenOfJob);
    //         job[lenOfJob] = '\0';
    //         jobList[numOfJobs] = makeJob(job);
    //         //jobList[numOfJobs] = job;
    //         currentStartOfJob = i+1;
    //         numOfJobs++;
    //     }
    // }

    /**
     * get first data
     * make Job
     * run Job
     * 
     * if second data
     * make Job
     * run Job
     * 
     */
    char* restOfCmd;
    char* tok = strtok_r(cmd, "|", &restOfCmd);
    Job** jobsMade= malloc(sizeof(Job*) * 2);
    int numOfJobs = 0;
    while(tok != NULL){
        if(tok[0] == ' ') tok += 1;//rmv white space from start
        if(tok[strlen(tok)] == ' ') tok[strlen(tok)] = '\0'; ////rmv white space from end
        //Job* j = makeJob(tok);
        jobsMade[numOfJobs] = makeJob(tok);
        if(jobsMade[numOfJobs] == NULL){
            fprintf(stderr, "mysh: could not parse command\n");
            return MYSH_EXIT_FAILURE;
        }
        printJob(jobsMade[numOfJobs]);
        //int returnStatus = runJob(job);
        // if(returnStatus == MYSH_EXIT_FAILURE){
        //     printf("Job Failed\n");
        // }
        // if(returnStatus == MYSH_EXIT_SUCCESS){
        //     printf("Job Succed\n");
        // }
        tok = strtok_r(NULL, "|", &restOfCmd);
        numOfJobs++;
    }

    int returnStatus = runJobs(jobsMade, numOfJobs);
    for(int i = 0; i < numOfJobs; i++){
        freeJob(jobsMade[i]);
    }
    if(returnStatus == MYSH_EXIT_SUCCESS){
        printf("Job Succed\n");
        return MYSH_EXIT_SUCCESS;
    }
    return MYSH_EXIT_FAILURE;
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
    printf("Num of args: %d\n", argc);
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
