#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <ctype.h>
#include "job.h"
#ifndef DEBUG
    #define DEBUG 1
#endif
#define BUFSIZE 256
#define MYSH_EXIT_SUCCESS 1
#define MYSH_EXIT_FAILURE 0
#define MYSH_EXIT_UNDEF 2

#define COND_END_INDEX 4

int mysh_errno = MYSH_EXIT_SUCCESS;

/**
 * @brief 
 * 
 * @param s 
 * @return int 1 if fully white space, 0 if not
 */
int strisempty(char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s))
      return 0;
    s++;
  }
  return 1;
}

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
}
int isBuiltInJob(Job* job){
    return strcmp(job->execPath, "cd") == 0 || strcmp(job->execPath, "pwd") == 0 || strcmp(job->execPath, "which") == 0;
}
int runBuiltInJob(Job* job){
    //duplicated stdin/stdout
    int exit_status = MYSH_EXIT_FAILURE;
    int dupedInfd = dup(STDIN_FILENO);
    int dupedOutfd = dup(STDOUT_FILENO);

    //setup redierection
    if(strcmp(job->inputReDirectPath, "") != 0){
        //input redircetion
        int fd = open(job->inputReDirectPath, O_RDONLY);
        if(fd != -1){
            dup2(fd, STDIN_FILENO);
        } else{
            fprintf(stderr, "mysh: %s: %s\n", strerror(errno), job->inputReDirectPath);
            exit_status = MYSH_EXIT_FAILURE;
        }
    }
    if(strcmp(job->outputReDirectPath, "") != 0){
        int fd = open(job->outputReDirectPath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
        if(fd != -1){
            dup2(fd, STDOUT_FILENO);
        } else{
            fprintf(stderr, "mysh: %s: %s\n", strerror(errno), job->outputReDirectPath);
            exit_status = MYSH_EXIT_FAILURE;
        }
    }
    if(strcmp(job->execPath, "cd") == 0){
        
    } else if (strcmp(job->execPath, "pwd") == 0){
        
    } else if (strcmp(job->execPath, "which") == 0){
        
    } 
    //reset in/out to terminal
    dup2(dupedInfd, STDIN_FILENO);
    dup2(dupedOutfd, STDOUT_FILENO);
    return exit_status;

}
int runJobs(Job** jobs, int numOfJobs){
    if(numOfJobs == 1){
        if(isBuiltInJob(job[0])){
            return runBuiltInJob(job[0]);
        }
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
        int fd[2];
        pipe(fd);
        if (fork() == 0) { //first child
            dup2(fd[1], STDOUT_FILENO);
            runJob(jobs[0]);
        }
        int child_status;
        wait(&child_status);
        if(child_status != 0)
            return MYSH_EXIT_FAILURE;
        close(fd[1]);

        if (fork() == 0) { //second child
            dup2(fd[0], STDIN_FILENO);
            runJob(jobs[1]);
        }
        int child_status2;
        wait(&child_status2);
        if(child_status2 != 0)
            return MYSH_EXIT_FAILURE;
        else
            return MYSH_EXIT_SUCCESS;
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
    //printf("CMD: |%s|---------------------------------\n", cmd);
    if(strisempty(cmd)) return MYSH_EXIT_UNDEF;
    if(strcmp(cmd, ""))
    if(strstr(cmd, "exit") != NULL){
        printf("mysh: exiting\n");
        exit(EXIT_SUCCESS);
    }
    if(cmd[strlen(cmd)-1] == '|'){
        fprintf(stderr, "mysh: could not parse command\n");
        return MYSH_EXIT_FAILURE;
    }
    char* restOfCmd;
    char* tok = strtok_r(cmd, "|", &restOfCmd);
    Job** jobsMade= malloc(sizeof(Job*) * 2);
    int numOfJobs = 0;
    while(tok != NULL && numOfJobs < 2){
        // if(tok[0] == ' ') tok += 1;//rmv white space from start
        // if(tok[strlen(tok)] == ' ') tok[strlen(tok)] = '\0'; ////rmv white space from end
        //printf("BTOK: |%s|\n", tok);
        while(isspace(tok[0])) tok += 1; //rmv white space from start
        //rmv white space from end
        //printf("outside: |%c|\n", tok[strlen(tok)-1]);
        int i = strlen(tok) - 1;
        while(isspace(tok[i])) {
            //printf("here: |%c|\n", tok[i]);
            i--;
        }
        tok[i+1] = '\0';

        //printf("ATOK: |%s|\n", tok);
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
        //printf("NTOK: |%s|\n", tok);
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
    printf("Job Failed\n");
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
void accept_line(char* line){
    if(strncmp(line, "then", COND_END_INDEX) == 0){
        printf("Checking then\n");
        if(mysh_errno == MYSH_EXIT_SUCCESS || mysh_errno == MYSH_EXIT_UNDEF){
            mysh_errno = accept_cmd_line(line+COND_END_INDEX+1);
        }
        else {
            //used "then", but prev cmd retured a error
            //so the new cmd is not run
        }
            
    }
    else if(strncmp(line, "else", COND_END_INDEX) == 0){
        printf("Checking else\n");
        if(mysh_errno == MYSH_EXIT_FAILURE || mysh_errno == MYSH_EXIT_UNDEF){
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
    int SHOWPROMPTS = 0;
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
                accept_line(line);
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
            accept_line(line);
        }
        if (SHOWPROMPTS) promptNextCMD();

    }

    printf("mysh: exiting\n");
    close(fd);
    return EXIT_SUCCESS;
}
