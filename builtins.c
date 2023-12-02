#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>
#include "builtins.h"
char * paths[] = {
    "/usr/local/bin",
    "/usr/bin",
    "/bin",
};
int isBuiltIn(char* cmdName){
    if(cmdName == NULL) return 0;
    return strcmp(cmdName, "cd") == 0 || strcmp(cmdName, "pwd") == 0 || strcmp(cmdName, "which") == 0;
}
// which is used to find the path to a command.
// It expects one argument, which is the name of a command.
// It should print the full path to the command.
// If the command is not found, it should print an error message and fail.

/**
 * @brief which prints nothing and fails if it is given the 
 *      wrong number of arguments, 
 *      or the name of a built-in, 
 *      or if the program is not found.
 * @param job 
 * @return int 
 */
int which(Job* job){
    if(job->numOfArgs == 1){
        fprintf(stderr, "%s: %s\n", job->execPath, "No enough arguments");
        return MYSH_EXIT_FAILURE;
    }
    else if(job->numOfArgs != 2){
        fprintf(stderr, "%s: %s\n", job->execPath, "Too many arguments");
        return MYSH_EXIT_FAILURE;
    }
    if(isBuiltIn(job->args[1])){
        fprintf(stderr, "%s: %s\n", job->args[1], "mysh built-in command");
        return MYSH_EXIT_FAILURE; 
    }
    for(int i = 0; i < NUMOFPATHS; i++){
        char* fullpath = my_acess(paths[i], job->args[1], strlen(job->args[1]));
        if(fullpath != NULL){
            printf("%s\n", fullpath);
            free(fullpath);
            return MYSH_EXIT_SUCCESS;
        }
    }
    // got here, so path is not found.
    fprintf(stderr, "%s: %s\n", job->args[1], "command not found");
    return EXIT_FAILURE;

    // char *path = getenv("PATH");
    // char *pathCopy = malloc(sizeof(char) * 256);
    // strcpy(pathCopy, path);
    // char *token = strtok(pathCopy, ":");
    // while(token != NULL){
    //     char *fullPath = malloc(sizeof(char) * 256);
    //     strcpy(fullPath, token);
    //     strcat(fullPath, "/");
    //     strcat(fullPath, programName);
    //     if(access(fullPath, F_OK) == 0){
    //         //return fullPath;
    //         return MYSH_EXIT_SUCCESS;
    //     }
    //     token = strtok(NULL, ":");
    // }
    // printf("%s: Command not found\n", programName);
    // //return NULL;
    // return MYSH_EXIT_FAILURE;
}

// cd is used to change the working directory.
// It expects one argument, which is a path to a directory.
// mysh should use chdir() to change its own directory.
// cd should print an error message and fail if it is given the wrong number of arguments, or if chdir() fails.

int cd(Job* job){
    if(job->numOfArgs == 1){
        fprintf(stderr, "%s: %s\n", job->execPath, "No enough arguments");
        return MYSH_EXIT_FAILURE;
    }
    else if(job->numOfArgs != 2){
        fprintf(stderr, "%s: %s\n", job->execPath, "Too many arguments");
        return MYSH_EXIT_FAILURE;
    }
    if(chdir(job->args[1]) == 0){
        return MYSH_EXIT_SUCCESS;
    } else {
        fprintf(stderr, "%s: %s: %s\n", job->execPath, strerror(errno), job->args[1]);
        return MYSH_EXIT_FAILURE;
    }

    // if(path == NULL){
    //     printf("cd: missing argument\n");
    //     return 0;
    // }
    // if(chdir(path) == -1){
    //     printf("cd: %s: No such file or directory\n", path);
    //     return 0;
    // }
    // return 1;
}

// pwd is used to print the current working directory.
// It takes no arguments.
// mysh should use getcwd() to get its current directory.

int pwd(Job* job){
    if(job->numOfArgs != 1){
        fprintf(stderr, "%s: %s\n", job->execPath, "Too many arguments");
        return MYSH_EXIT_FAILURE;
    }
    char* fulldir = getcwd(NULL, 0);
    if(fulldir != NULL){
        printf("%s\n", fulldir);
        free(fulldir);
        printf("PWD: here\n");
        return MYSH_EXIT_SUCCESS;
    } else{
        return MYSH_EXIT_FAILURE;
    }

    // char *cwd = malloc(sizeof(char) * 256);
    // if(getcwd(cwd, 256) == NULL){
    //     printf("pwd: getcwd() error\n");
    //     return NULL;
    // }
    // return cwd;
}