#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/errno.h>
#include <string.h>
#include "builtins.h"
/**
 * @brief This holdes the paths to check for bare names
 * 
 */
char * paths[] = {
    "/usr/local/bin",
    "/usr/bin",
    "/bin",
};
/**
 * @brief This function checks if a give cmdName matches the builtin funcs
 * 
 * @param cmdName the name to test
 * @return int 0 if false, 1 if true
 */
int isBuiltIn(char* cmdName){
    if(cmdName == NULL) return 0;
    return strcmp(cmdName, "cd") == 0 || strcmp(cmdName, "pwd") == 0 || strcmp(cmdName, "which") == 0;
}

/**
 * @brief Which is used to find the path to a command. It accepts 1 argument
 * which prints nothing and fails if it is given the wrong number of arguments, or the name of a built-in, or if the program is not found.
 * @param job The Job to run
 * @return int 1 if job runs, 0 if not
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
}
/**
 * @brief cd is used to change the working directory. It accepts 1 argument.
 * It prints a error if it is given the wrong number of arguments, or changing the dir failed
 * 
 * @param job The Job to run
 * @return int 1 if job runs, 0 if not
 */
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
}

/**
 * @brief pwd is used to print the current working directory. It takes no arguments
 * It prints a error if it is given the wrong number of arguments.
 * 
 * @param job 
 * @return int 
 */
int pwd(Job* job){
    if(job->numOfArgs != 1){
        fprintf(stderr, "%s: %s\n", job->execPath, "Too many arguments");
        return MYSH_EXIT_FAILURE;
    }
    char* fulldir = getcwd(NULL, 0);
    if(fulldir != NULL){
        printf("%s\n", fulldir);
        free(fulldir);
        return MYSH_EXIT_SUCCESS;
    } else{
        return MYSH_EXIT_FAILURE;
    }
}