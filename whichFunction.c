#include <stdio.h>
#include <stdlib.h>
#include <sys/unistd.h>

// which is used to find the path to a command.
// It expects one argument, which is the name of a command.
// It should print the full path to the command.
// If the command is not found, it should print an error message and fail.

char *which(char* command){
    char *path = getenv("PATH");
    char *pathCopy = malloc(sizeof(char) * 256);
    strcpy(pathCopy, path);
    char *token = strtok(pathCopy, ":");
    while(token != NULL){
        char *fullPath = malloc(sizeof(char) * 256);
        strcpy(fullPath, token);
        strcat(fullPath, "/");
        strcat(fullPath, command);
        if(access(fullPath, F_OK) == 0){
            return fullPath;
        }
        token = strtok(NULL, ":");
    }
    printf("%s: Command not found\n", command);
    return NULL;
}