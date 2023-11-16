#include "job.h"
#include <stdio.h>

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
 * @return Job* 
 */
Job *makeJob(char* jobCmd){
    //deals with wildcards, input/output redirection, and execPath finder(bare names)
    //also make sepereate .c files to deal with cd, pwd, which, bc these 3 cmds, we are
    //suppose to implement.
    return NULL;
}


