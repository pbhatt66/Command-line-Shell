Pranay Bhatt: pdb91
Dharmik Patel: dsp187

MakeFile:
- Sometimes if the "make" command does not work, or throws errors, then:
    change "MakeFile" to "MakeFile" or change "Makefile" to "MakeFile"
- make mysh:
    run this cmd to make a excuatble file called "mysh"
    to run the file, use "./mysh"
- make test:
    run this cmd to test mysh
    this makes mysh excuatble, and then runs ./mysh with .sh file
- B flag: include -B in any of the cmds above to force rebuild the excutable file
- DEBUG:
    edit the Makefile and change line 2 to "DEBUG = -D DEBUG=1"
    this prints out some usefull debug inforamtion
- make clean:
    run this to remove all .o, mysh(excutable), .dSYM files.
    use this before commiting/merging with github


Overview:
- Interactive Mode:
- Batch Mode:

Important Notes:
- exiting:
    - exit with or without any args, with or without pipes
        will exit mysh
    - the token "exit" in a argument name(ex: "echo exit")
        will not exit mysh
- all white space is ingnored, even btw arguments
- the follwing commands mean the same thing.
    - |ls|echo man|
    - |ls   |echo         man|
    - |ls|   echo man|
    - |ls        |      echo man|
    - |         ls        |      echo man        |
- use fflush bc printf() does not print until read() stops blocking
- only use single pipes. no mutli-pipes cmds
- tab completion, and up arrow for last cmd does not work.
- to see how the builtins work, go to Implemntation->builtins.c


Implemntation
- job.c:
    - represents and abstract data type called Job
    - Job:
        char* execPath:
            - if builtin: then just builtin name
            - if non-builtin: full path gotten from acess. check /usr/local/bin, /usr/bin, and /bin, in that order
            - if contains a /, then a path already. assume its correct
            - if non of the above 3, then NULL
        char** args:
            - prorgram name -> arg1.... -> NULL
            - expanded using wildcards
                - only 1 '*' allowed
        int numOfArgs:
            - len of the args array
        char* inputReDirectPath:
            - arg followed by '<'
            - "" if there is no '<'
        char* outputReDirectPath:
            - arg followed by '>'
            - "" if there is no '>'
- builtins.c:
    - holds the implementations for cd, which and pwd.
    - cd:
        used to change the working directory
        accepts Job* as param
        if Job->numOfArgs == 1: error: not enough args and return MYSH_EXIT_FAILURE
        if Job->numOfArgs != 2: error: too many args and return MYSH_EXIT_FAILURE
        if passes above 2 cases:
            then try to chdir to job->args[1]:
                if sucess when chdir: return MYSH_EXIT_SUCCESS 
                else: error from errno, and return MYSH_EXIT_FAILURE 
    - which:
        used to find the path to a program name
        accepts Job* as param
        if Job->numOfArgs == 1: error: not enough args and return MYSH_EXIT_FAILURE
        if Job->numOfArgs != 2: error: too many args and return MYSH_EXIT_FAILURE
        if its a builtin name: error: {program name}: mysh built-in command and return MYSH_EXIT_FAILURE
        if passes above 2 cases:
            then try to get the path using acess(). check /usr/local/bin, /usr/bin, and /bin, in that order.
                if sucess, print fullpath, return MYSH_EXIT_SUCCESS.
            if that fails: error: command not foun and return MYSH_EXIT_FAILURE
    - pwd:
        used to print the working dir
        accepts Job* as param
        if Job->numOfArgs != 1: error: too many args and return MYSH_EXIT_FAILURE
        try to get dir from getcwd(). 
            if getcwd() sucess: print working dir and return MYSH_EXIT_SUCCESS.
            else, then report error and return MYSH_EXIT_FAILURE

- mysh.c:
    - this is the program launcher
    - if in batch mode, then it sets stdin to the given file
    - keeps a global variable called mysh_errno to keep track of prev exist status to  deal with “then” “else” cmds lines
    - it reads from stdin each cmd line, and then for each cmd:
            - trims the cmd from leading and ending white space using isspace()
            - splits the cmd into subcmds at pipes if any
            - for each subcmd
                - PS: "Job" and "subcmd" are interchangable, mean the same thing
                - make a Job* to hold the data for use
                - adds the Job* to a list of jobs: Job**
            - now it is time to run the job(s).
                - if single job:
                    -if builtin:
                        - store backup stdin and stdout using dup()
                        - set up input/output redierection with dup2()
                        - run builtin
                        - reset stdin and stdout using dup2
                        run on same process
                    -if non-builtin
                        -fork()->set up input/output redierection with dup2()->exec()
                        -then wait(), and get the return status to set mysh_errno
                - if pipe:
                    - non-builtin | non-builtin
                        - setup pipe using pipe() and dup2()
                        - run each job as a single non-builtin job
                    - builtin | non-builtin OR non-builtin | builtin
                        - OFFICE HOURS
                    - builtin | builtin
                        - OFFICE HOURS
- test.sh:
    - holds a list of cmds to test mysh
    - run with ./mysh test.sh or make test
    - see test section
- librarys:
    - used arraylist.c from class 
Tests:

