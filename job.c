#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "job.h"



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
    Job* job = malloc(sizeof(Job));
    job->execPath = malloc(sizeof(char) * 256);
    job->args = malloc(sizeof(char *) * 256);
    for(int i = 0; i < 256; i++){
        job->args[i] = malloc(sizeof(char) * 256);
    }
    job->inputReDirectPath = malloc(sizeof(char) * 256);
    job->outputReDirectPath = malloc(sizeof(char) * 256);

    // initialize all fields to null originally
    job->execPath[0] = '\0';
    for(int i = 0; i < 256; i++){
        job->args[i][0] = '\0';
    }
    job->inputReDirectPath[0] = '\0';
    job->outputReDirectPath[0] = '\0';

    int jobCmdLen = strlen(jobCmd);
    
    //first, find the execPath
    int i = 0;
    while(jobCmd[i] != ' ' && jobCmd[i] != '\0'){
        job->execPath[i] = jobCmd[i];
        i++;
    }
    job->execPath[i] = '\0';
    //now, find the args
    int argsIndex = 0;
    while (jobCmd[i] != '\0'){
        if(jobCmd[i] == ' '){
            i++;
            continue;
        }
        else if(jobCmd[i] == '<'){
            i++;
            while(jobCmd[i] == ' '){
                i++;
            }
            int j = 0;
            while(jobCmd[i] != ' ' && jobCmd[i] != '\0'){
                job->inputReDirectPath[j] = jobCmd[i];
                i++;
                j++;
            }
            job->inputReDirectPath[j] = '\0';
        }
        else if(jobCmd[i] == '>'){
            i++;
            while(jobCmd[i] == ' '){
                i++;
            }
            int j = 0;
            while(jobCmd[i] != ' ' && jobCmd[i] != '\0'){
                job->outputReDirectPath[j] = jobCmd[i];
                i++;
                j++;
            }
            job->outputReDirectPath[j] = '\0';
        }
        else{
            int j = 0;
            while(jobCmd[i] != ' ' && jobCmd[i] != '\0'){
                job->args[argsIndex][j] = jobCmd[i];
                i++;
                j++;
            }
            job->args[argsIndex][j] = '\0';
            argsIndex++;
        }
            
    }

    return job;

}

// free the job struct
void freeJob(Job* job){
    free(job->execPath);
    for(int i = 0; i < 256; i++){
        free(job->args[i]);
    }
    free(job->args);
    free(job->inputReDirectPath);
    free(job->outputReDirectPath);
    free(job);
}


