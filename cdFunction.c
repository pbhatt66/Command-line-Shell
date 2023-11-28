#include <stdio.h>
#include <stdlib.h>

// cd is used to change the working directory.
// It expects one argument, which is a path to a directory.
// mysh should use chdir() to change its own directory.
// cd should print an error message and fail if it is given the wrong number of arguments, or if chdir() fails.

int cd(char* path){
    if(path == NULL){
        printf("cd: missing argument\n");
        return 0;
    }
    if(chdir(path) == -1){
        printf("cd: %s: No such file or directory\n", path);
        return 0;
    }
    return 1;
}