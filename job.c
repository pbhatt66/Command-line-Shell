#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <glob.h>
#include <dirent.h>
#include <fnmatch.h>
#include <string.h>
#include <sys/stat.h>
#include "job.h"
#include "arraylist.h"
#include "builtins.h"

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
 * @brief This utility function checks if a directory entry is . or .. or hiden(starts with .)
 * Serves as a base case for the recursion
 * 
 * @param name The name of the file
 * @return int 1 if it is, 0 if it is not
 */
int isItSelfOrParrentDirOrHidden(char* name){

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0){
        return 1;
    }
    if(name[0] == '.'){
        return 1;
    }
    return 0;
}

void handleWildcards(char* arg, arraylist_t* argList) {
    char* slash = strchr(arg, '/');
    char* dir_part;
    char* file_part;

    if (slash) {
        *slash = '\0';
        dir_part = arg;
        file_part = slash + 1;
    }
    else {
        dir_part = ".";
        file_part = arg;
    }
    DIR *dir = opendir(dir_part);
    if (dir == NULL) {
        fprintf(stderr, "Error: opendir() failed\n");
        return;
    }
    struct dirent *entry;
    int found = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (isItSelfOrParrentDirOrHidden(entry->d_name)) {
            continue;
        }
        struct stat stats;
        char* temp;
        char* pathCopy = strdup(entry->d_name);
        if (dir_part[0] != '.') {
            temp = malloc(sizeof(char) * (strlen(dir_part) + strlen(pathCopy) + 2));
            // printf("dir_part: |%s|\n", dir_part);
            strcpy(temp, dir_part);
            strcat(temp, "/");
            strcat(temp, pathCopy);
            // al_push(argList, temp);
        }
        else {
            temp = entry->d_name;
        }
        // strcpy(temp, entry->d_name);
        // strcat(temp, file_part);
        // printf("temp: |%s|\n", temp);
        if(stat(temp, &stats) == 0){
            if (!S_ISREG(stats.st_mode)) {
                // printf("here\n");
                continue;
            }
        }
        if (fnmatch(file_part, entry->d_name, 0) == 0) {
            // char* pathCopy = strdup(entry->d_name);
            // printf("dir_path: |%s|\n", dir_part);
            if (dir_part[0] != '.') {
                char* newPath = malloc(sizeof(char) * (strlen(dir_part) + strlen(pathCopy) + 2));
                // printf("dir_part: |%s|\n", dir_part);
                strcpy(newPath, dir_part);
                strcat(newPath, "/");
                strcat(newPath, pathCopy);
                al_push(argList, newPath);
            }
            else {
                al_push(argList, pathCopy);
            }
            found = 1;
        }
    }
    closedir(dir);
    if (!found) {
        char* pathCopy = strdup(arg);
        al_push(argList, pathCopy);
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
    printf("Parsing: |%s|\n", bareName);
    arraylist_t* argList = al_create(1);
    if(strchr(bareName, '/')){
        job->execPath = bareName;
    } 
    else if(isBuiltIn(bareName)){
        job->execPath = bareName;
    }
    else{
        for(int i = 0; i < NUMOFPATHS; i++){
            job->execPath = my_acess(paths[i], bareName, lenOfBareName);
            if(job->execPath != NULL) break;
        }
        free(bareName);
    }
    // }

    //make argList
    // arraylist_t* argList = al_create(1);
    int i = 0;
    printf("jobCmd: |%s|\n", jobCmd);
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
            printf("|%s|\n", currArg);

            if (strchr(currArg, '*') != NULL) {
                // glob_t glob_result;
                // memset(&glob_result, 0, sizeof(glob_result));

                // int return_value = glob(currArg, GLOB_TILDE, NULL, &glob_result);
                // if (return_value != 0) {
                //     globfree(&glob_result);
                //     fprintf(stderr, "Error: glob() failed\n");
                //     return NULL;
                // }
                // for (size_t i = 0; i < glob_result.gl_pathc; i++) {
                //     char* pathCopy = strdup(glob_result.gl_pathv[i]);
                //     al_push(argList, pathCopy);
                // }
                // globfree(&glob_result);
                handleWildcards(currArg, argList);
            } else {
            al_push(argList, currArg);
            // argsIndex++;
            }
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