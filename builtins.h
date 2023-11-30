#ifndef BUILTINS
#define BUILTINS

#include "mysh_lib.h"
#include "job.h"

int cd(Job* job);
int which(Job* job);
int pwd(Job* job);
int isBuiltIn(char* cmdName);

#endif

