#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include "builtins.h"
#include "job.h"
#include "mysh_lib.h"

#define BUFSIZE 256
#define SLEEPLEN 5

int mysh_errno = MYSH_EXIT_SUCCESS;

/**
 * @brief Goes through each character in the str, and makes sure it is not blank space
 * 
 * @param s The str to check
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
/**
 * @brief Removes leading and trailing white space from a str
 * 
 * @param s The str to trim
 * @return char* The same ptr to s
 */
char* removeLeadingTrailingSpace(char *s){
    //rmv white space from start
    while(isspace(s[0])) {
        s += 1;
    } 
    //rmv white space from end
    int i = strlen(s) - 1;
    while(isspace(s[i])) {
        i--;
    }
    s[i+1] = '\0';
    return s;
}

/**
 * @brief This runs a non-builtin job
 * 
 * @param job The Job to run
 * @return int 0 if fail, 1 if success
 */
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
/**
 * @brief This runs a builtin job. This also resets the input/output redir once done
 * 
 * @param job The Job to run
 * @return int 0 if fail, 1 if success
 */
int runBuiltInJob(Job* job){
    int exit_status = MYSH_EXIT_FAILURE;
    int dupedInfd = dup(fileno(stdin));
    int dupedOutfd = dup(fileno(stdout));
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
    //run job
    if(strcmp(job->execPath, "cd") == 0){
        exit_status = cd(job);
    } else if (strcmp(job->execPath, "pwd") == 0){
        exit_status = pwd(job);
    } else if (strcmp(job->execPath, "which") == 0){
        exit_status = which(job);
    } 
    //reset stdin and out
    if(strcmp(job->inputReDirectPath, "") != 0){
        dup2(dupedInfd, STDIN_FILENO);
    } 
    if(strcmp(job->outputReDirectPath, "") != 0){
        dup2(dupedOutfd, STDOUT_FILENO);
    } 
    close(dupedInfd);
    close(dupedOutfd);
    return exit_status;

}
/**
 * @brief This runs a list of jobs.
 * 
 * @param jobs List of jobs, Max 2
 * @param numOfJobs Num of jobs, Max 2
 * @return int int 0 if fail, 1 if success
 */
int runJobs(Job** jobs, int numOfJobs){
    if(numOfJobs == 1){
        if(isBuiltIn(jobs[0]->execPath)){
            return runBuiltInJob(jobs[0]);
        }
        if (fork() == 0) {
            runJob(jobs[0]);
        }
        int child_status;
        wait(&child_status);
        return child_status;
    }
    else{
        int fd[2];
        pipe(fd);
        int childstatus1 = MYSH_EXIT_UNDEF;
        int childstatus2 = MYSH_EXIT_UNDEF;
        if(isBuiltIn(jobs[0]->execPath)){
            int stdoutbackup = dup(STDOUT_FILENO);
            dup2(fd[1], STDOUT_FILENO); //pipe setup
            childstatus1 = runBuiltInJob(jobs[0]);
            dup2(stdoutbackup, STDOUT_FILENO); //reset
            close(stdoutbackup);
        } else {
            if (fork() == 0) { //first child
                dup2(fd[1], STDOUT_FILENO); //pipe setup
                runJob(jobs[0]);
            }
            wait(&childstatus1);
        }
        
        close(fd[1]);
        
        if(isBuiltIn(jobs[1]->execPath)){
            int stdinbackup = dup(STDIN_FILENO);
            dup2(fd[0], STDIN_FILENO); //pipe setup
            childstatus2 = runBuiltInJob(jobs[1]);
            dup2(stdinbackup, STDIN_FILENO); //reset
            close(stdinbackup);
        } else {
            if (fork() == 0) { //second child
                dup2(fd[0], STDIN_FILENO); //pipe setup
                runJob(jobs[1]);
            }
            wait(&childstatus2);
        }
        close(fd[0]);
        return childstatus2;
    }
}

/**
 * @brief This is the main utilty function. It takes in a cmd line
 * and then parse through it. Calls build job to make the jobs if pipes are included.
 * Once the array of jobs are built, it will call loop through each job, and deal with
 * making pipes, input/output redirects, and conditionals.
 * 
 * @param cmd 1 cmd token until new line
 * @return int MYSH_EXIT_SUCCESS | MYSH_EXIT_FAILURE
 */
int accept_cmd_line(char *cmd) {
    if(strisempty(cmd)) return MYSH_EXIT_UNDEF;
    if(strcmp(cmd, ""))
    cmd = removeLeadingTrailingSpace(cmd);
    if(cmd[strlen(cmd)-1] == '|' || cmd[0] == '|'){ //if ends or starts in |
        fprintf(stderr, "mysh: could not parse command\n");
        return MYSH_EXIT_FAILURE;
    }
    char* restOfCmd;
    char* tok = strtok_r(cmd, "|", &restOfCmd);
    Job** jobsMade= malloc(sizeof(Job*) * 2);
    int numOfJobs = 0;
    while(tok != NULL && numOfJobs < 2){
        tok = removeLeadingTrailingSpace(tok);
        if(strncmp(tok, "exit", COND_END_INDEX) == 0){
            printf("mysh: exiting\n");
            exit(EXIT_SUCCESS);
        }
        jobsMade[numOfJobs] = makeJob(tok);
        if(jobsMade[numOfJobs] == NULL){
            fprintf(stderr, "mysh: could not parse command\n");
            return MYSH_EXIT_FAILURE;
        }
        
        //printJob(jobsMade[numOfJobs]);
        tok = strtok_r(NULL, "|", &restOfCmd);
        numOfJobs++;
    }

    int returnStatus = runJobs(jobsMade, numOfJobs);
    for(int i = 0; i < numOfJobs; i++){
        freeJob(jobsMade[i]);
    }
    if(returnStatus == MYSH_EXIT_SUCCESS){
        //printf("Job Succed\n");
        return MYSH_EXIT_SUCCESS;
    }
    //printf("Job Failed\n");
    return MYSH_EXIT_FAILURE;
}
/**
 * @brief Prints "mysh> " using write
 */
void promptNextCMD(){
    write(STDOUT_FILENO, "mysh> ", 7);
    // printf("mysh> ");
    // fflush(stdout);
}
/**
 * @brief This checks for then and else statments.
 * 
 * @param line The cmd line until new line
 */
void accept_line(char* line){
    if(strncmp(line, "then", COND_END_INDEX) == 0){
        // printf("Checking then\n");
        if(mysh_errno == MYSH_EXIT_SUCCESS || mysh_errno == MYSH_EXIT_UNDEF){
            mysh_errno = accept_cmd_line(line+COND_END_INDEX+1);
        }
        else {
            //used "then", but prev cmd retured a error
            //so the new cmd is not run
        }
            
    }
    else if(strncmp(line, "else", COND_END_INDEX) == 0){
        // printf("Checking else\n");
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
