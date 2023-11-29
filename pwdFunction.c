#include <stdio.h>
#include <stdlib.h>

// pwd is used to print the current working directory.
// It takes no arguments.
// mysh should use getcwd() to get its current directory.

char *pwd(){
    char *cwd = malloc(sizeof(char) * 256);
    if(getcwd(cwd, 256) == NULL){
        printf("pwd: getcwd() error\n");
        return NULL;
    }
    return cwd;
}