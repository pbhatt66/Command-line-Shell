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

/**
 * @brief This function checks if the program exists in the given path
 * 
 * @param pathToCheck The path to check in.
 * @param bareName The bare name of the program
 * @param lenOfBareName The len of the bare name
 * @return char* The fullpath, or NULL if not found
 */
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
 * @brief This utility function checks if a
 * directory entry is . or .. or hiden(starts with .)
 * Makes sure we do not check for wildcard expansion with hidden files, or . or ..
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

/**
 * @brief This function handles wildcards. Up to the client to insure that
 * the function is only called after detecting a proper *.
 * 
 * @param arg The wildcard pattern to expand
 * @param argList The arraylist to add to
 * @return int 0 if no matches found. 1 if matches found
 */
int  handleWildcards(char* arg, arraylist_t* argList) {
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
        return 0;
    }
    struct dirent *entry;
    int found = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (isItSelfOrParrentDirOrHidden(entry->d_name)) {
            continue;
        }
        struct stat stats;
        char* temp;
        int sizeofpathcpy = strlen(entry->d_name);
        //printf("Size of %lu\n", strlen(entry->d_name));
        char* pathCopy = malloc(sizeof(char) * (sizeofpathcpy + 1));
        strcpy(pathCopy, entry->d_name);
        //char* pathCopy = strdup(entry->d_name);
        int malloced = 0;
        if (dir_part[0] != '.') {
            temp = malloc(sizeof(char) * (strlen(dir_part) + strlen(pathCopy) + 2));
            malloced = 1;
            strcpy(temp, dir_part);
            strcat(temp, "/");
            strcat(temp, pathCopy);
        }
        else {
            temp = entry->d_name;
        }
        if(stat(temp, &stats) == 0){
            if (malloced) free(temp);
            if (!S_ISREG(stats.st_mode)) {
                continue;
            }
        } else {
            if (malloced) free(temp);
        }
        if (fnmatch(file_part, entry->d_name, 0) == 0) {
            if (dir_part[0] != '.') {
                char* newPath = malloc(sizeof(char) * (strlen(dir_part) + strlen(pathCopy) + 2));
                strcpy(newPath, dir_part);
                strcat(newPath, "/");
                strcat(newPath, pathCopy);
                al_push(argList, newPath);
                free(pathCopy);
            }
            else {
                al_push(argList, pathCopy);
            }
            found = 1;
        } else {
            free(pathCopy);
        }
    }
    closedir(dir);
    if (!found) {
        return 0;
    }
    return 1;
}   
/**
 * @brief Make sure a * happens after the last / in the path name
 * 
 * @param name The wildcard pattern
 * @return int 1 if true, 0 if false
 */
int isRegexInLastPath(char *name){
    char* locOfLastSlash = strrchr(name, '/');
    char* locOfFirstAshtrik = strchr(name, '*');
    if(locOfFirstAshtrik == NULL){
        // if no *
        return 0;
    }
    if (locOfLastSlash == NULL) {
        // if no / and at this point, the name must have a *
        return 1;
    }
    if (locOfFirstAshtrik > locOfLastSlash) {
        // the name contains a * and a slash
        // if the astrik occurs after the slash
        return 1;
    } else {
        return 0;
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
 * @return Job* A job if succesfull or NULL if parsing error
 */
Job *makeJob(char* jobCmd){
    Job* job = malloc(sizeof(Job));
    job->inputReDirectPath = malloc(sizeof(char) * 256);
    job->outputReDirectPath = malloc(sizeof(char) * 256);
    job->inputReDirectPath[0] = '\0';
    job->outputReDirectPath[0] = '\0';
   
    if(strlen(jobCmd) == 0) return NULL;
    char* endOfBareName = strpbrk(jobCmd, " \t\r"); //get a ptr to first ' ' or '\r' or '\t' in jobCmd
    if(endOfBareName == NULL) endOfBareName = jobCmd + strlen(jobCmd);
    size_t lenOfBareName = endOfBareName - jobCmd; //len w/o terminator
    char* bareName = malloc((lenOfBareName + 1) * sizeof(char)); //malloc lenOfBareName + terminator
    strncpy(bareName, jobCmd, lenOfBareName); //cpy barename
    bareName[lenOfBareName] = '\0';           //append terminator

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
    int i = 0;
    //printf("jobCmd: |%s|\n", jobCmd);
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
            if(isRegexInLastPath(currArg)) {
                if(!handleWildcards(currArg, argList)) {
                    al_push(argList, currArg);
                }
                else {
                    free(currArg);
                }
            } else {
                al_push(argList, currArg);
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
        free(arg);
    }
    job->args[job->numOfArgs] = NULL;
    al_destroy(argList);
    return job;
}

/**
 * @brief This free a given Job
 * 
 * @param job The job to free
 */
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

/**
 * @brief This prints a given Job
 * 
 * @param job The job to print
 */
void printJob(Job* job){
    printf("Job: \n");
    printf("\tExec: %s\n", job->execPath);
    printf("\tArgs: ");
    for(int i = 0; i < job->numOfArgs+1; i++){
        printf("|%s|", job->args[i]);
    }
    printf("\n");
    printf("\tInRe: %s\n", job->inputReDirectPath);
    printf("\tOuRe: %s\n", job->outputReDirectPath);
}