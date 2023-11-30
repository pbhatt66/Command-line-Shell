#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "job.h"
#include "arraylist.h"

#define NUMOFPATHS 3
// char* path1 = "/usr/local/bin";
// char* path2 = "/usr/bin";
// char* path3 = "/bin";



char * paths[] = {
    "/usr/local/bin",
    "/usr/bin",
    "/bin",
};

char* my_acess(char* pathToCheck, char* bareName, size_t lenOfBareName){
    size_t lenOfpath1PlusBareName = strlen(pathToCheck) + 1 + lenOfBareName; //len w/o terminator
    char* path1PlusBareName = malloc(lenOfpath1PlusBareName + 1);
    snprintf(path1PlusBareName, lenOfpath1PlusBareName + 1, "%s/%s", pathToCheck, bareName);
    if(access(path1PlusBareName, F_OK) == 0){
        return path1PlusBareName;
    }
    else{
        free(path1PlusBareName);
        return NULL;
    }
}

/**
 * @brief This takes in the string job cmd, and returns a Job Struct pointer
 * Example Inputs/Outputs: 
 * Input1:
 * "foo < bar baz"
 * Output1:
 * ptr to Job:
 *      execPath = foo
 *      args = {foo, baz}
 *      inputReDirectPath = "baz"
 *      outputReDirectPath = "\0" (NULL)
 * 
 * 
 * Input2:
 * "foo bar > baz "
 * Output2:
 * ptr to Job:
 *      execPath = foo
 *      args = {foo, bar}
 *      inputReDirectPath = "\0" (NULL)
 *      outputReDirectPath = "baz"
 * 
 * Input3:
 * " quux *.txt > spam"
 * Output3:
 * ptr to Job:
 *      execPath = quux
 *      args = {quux, a.txt,...(all .txt files in current dir)}
 *      inputReDirectPath = "spam"
 *      outputReDirectPath = "\0" (NULL)
 * 
 * There will be no pipes, or conditinals in the input string
 * @param jobCmds 
 * @return Job* | NULL if parsing error
 */
Job *makeJob(char* jobCmd){
    //deals with wildcards, input/output redirection, and execPath finder(bare names)
    //also make sepereate .c files to deal with cd, pwd, which, bc these 3 cmds, we are
    //suppose to implement.
    Job* job = malloc(sizeof(Job));
    //job->execPath = malloc(sizeof(char) * 256);
    //job->args = malloc(sizeof(char *) * 256);
    // for(int i = 0; i < 256; i++){
    //     job->args[i] = malloc(sizeof(char) * 256);
    // }
    job->inputReDirectPath = malloc(sizeof(char) * 256);
    job->outputReDirectPath = malloc(sizeof(char) * 256);

    // initialize all fields to null originally
    //job->execPath[0] = '\0';
    // for(int i = 0; i < 256; i++){
    //     job->args[i][0] = '\0';
    // }
    job->inputReDirectPath[0] = '\0';
    job->outputReDirectPath[0] = '\0';

    //int jobCmdLen = strlen(jobCmd);
   
    if(strlen(jobCmd) == 0) return NULL;
    char* endOfBareName = strpbrk(jobCmd, " \t\r"); //get a ptr to first ' ' or '\r' or '\t' in jobCmd
    if(endOfBareName == NULL) endOfBareName = jobCmd + strlen(jobCmd);
    size_t lenOfBareName = endOfBareName - jobCmd; //len w/o terminator
    char* bareName = malloc((lenOfBareName + 1) * sizeof(char)); //malloc lenOfBareName + terminator
    strncpy(bareName, jobCmd, lenOfBareName); //cpy barename
    bareName[lenOfBareName] = '\0';           //append terminator
    //printf("Parsing: |%s|\n", bareName);
    if(strchr(bareName, '/')){
        job->execPath = bareName;
    }
    else{
        for(int i = 0; i < NUMOFPATHS; i++){
            job->execPath = my_acess(paths[i], bareName, lenOfBareName);
            if(job->execPath != NULL) break;
        }
        free(bareName);
    }

    //make argList
    arraylist_t* argList = al_create(1);
    int i = 0;
    // int argsIndex = 0;
    while (jobCmd[i] != '\0'){
        if(isspace(jobCmd[i])){
            i++;
            continue;
        }
        else if(jobCmd[i] == '<'){
            i++;
            while(isspace(jobCmd[i]) || jobCmd[i] == '<' || jobCmd[i] == '>'){
                if(jobCmd[i] == '>') return NULL;
                i++;
            }
            int j = 0;
            if(jobCmd[i] == '\0'){
                return NULL;
            }
            while(!isspace(jobCmd[i]) && jobCmd[i] != '\0'){
                job->inputReDirectPath[j] = jobCmd[i];
                i++;
                j++;
            }

            job->inputReDirectPath[j] = '\0';
        }
        else if(jobCmd[i] == '>'){
            i++;
            while(isspace(jobCmd[i]) || jobCmd[i] == '>' || jobCmd[i] == '<'){
                if(jobCmd[i] == '<') return NULL;
                i++;
            }
            if(jobCmd[i] == '\0'){
                return NULL;
            }
            int j = 0;
            while(!isspace(jobCmd[i]) && jobCmd[i] != '\0'){
                job->outputReDirectPath[j] = jobCmd[i];
                i++;
                j++;
            }
            job->outputReDirectPath[j] = '\0';
        }
        else{
            char* currArg = malloc(sizeof(char)*256);
            int j = 0;
            while(!isspace(jobCmd[i]) && jobCmd[i] != '\0'){
                currArg[j] = jobCmd[i];
                i++;
                j++;
            }
            currArg[j] = '\0';
            //printf("|%s|\n", currArg);
            al_push(argList, currArg);
            // argsIndex++;
        }
            
    }
    job->numOfArgs = al_length(argList);
    job->args = malloc(sizeof(char*) * (job->numOfArgs + 1)); //+1 bc execv needs NULL terminated array
    for(int i = 0; i < job->numOfArgs; i++){
        char* arg;
        al_pop(argList, &arg);
        job->args[i] = malloc((sizeof(char) * strlen(arg)) + 1);
        strcpy(job->args[i], arg);
    }
    job->args[job->numOfArgs] = NULL;
    al_destroy(argList);
    return job;
}

// free the job struct
void freeJob(Job* job){
    free(job->execPath);
    for(int i = 0; i < job->numOfArgs; i++){
        free(job->args[i]);
    }
    free(job->args);
    free(job->inputReDirectPath);
    free(job->outputReDirectPath);
    free(job);
}
void printJob(Job* job){
    printf("Job: \n");
    printf("\tExec: %s\n", job->execPath);
    printf("\tArgs: ");
    for(int i = 0; i < job->numOfArgs+1; i++){
        printf("|%s|", job->args[i]);
    }
    // while (strcmp(job->args[i], ""))
    // {
    //     printf("|%s|", job->args[i]);
    //     i++;
    // }
    printf("\n");
    printf("\tInRe: %s\n", job->inputReDirectPath);
    printf("\tOuRe: %s\n", job->outputReDirectPath);
}