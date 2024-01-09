This is our implementation of the Linux Command Line called "mysh"

MakeFile:
- Sometimes if the "make" command does not work, or throws errors, then:
    change "MakeFile" to "MakeFile" or change "Makefile" to "MakeFile"
- make mysh:
    run this cmd to make a excuatble file called "mysh"
    to run the file, use "./mysh"
- make testJobMaking:
    run this cmd to test that we are making a job properly.
    look at Test->Test Interactive Mode section
- make batchmodetest:
    run this cmd to test the test.sh file in batch mode
    this will make the sumFunction and squareFunction excutable files that are used in our test case, then it will run the test.sh file
- make removeTXTFiles:
    run this cmd to remove the .txt files that were created when running the test.sh file in batch mode
    to use: "make removeTXTFiles"
- B flag: include -B in any of the cmds above to force rebuild the excutable file
- DEBUG:
    edit the Makefile and change line 2 to "DEBUG = -D DEBUG=1"
    this prints out some usefull debug inforamtion
- make clean:
    run this to remove all .o, mysh(excutable), .dSYM files.
    use this before commiting/merging with github


Overview:
- Interactive Mode:
    - Used to run 1 cmd at a time
- Batch Mode:
    - Used to run all the given cmds one after another, until exit cmd is reached
    - or will stop at EOF.

Important Notes:
- using buffered cmds like "less"
    on ilab, when you reach the end of file, and still countine to try to scroll, in the background
    random characters might be printed to mysh. 
    mysh will throw a error after quiting the less cmd. 
    there is nothing we can do, because the random characters print before we quit less. so we are still
    in the exec().
    We thought the stdout of the less cmd was wrong, but we compared it the actual less running on a
    normal terminal, and stdout was still set to the tty#.
- testing batchmode with our .sh file:
    use the cmd in the Makefile. "make batchmodetest"
- quotes:
    mysh does not deal with quotes. there are just treated as normal characters
- escaped space: 
    mysh does not deal with spaces in file names or dir. 
    in a normal terminal they are dealt with by escaping the space:('\ ')
    but mysh does not bc it is not reuqired
- conditinals:
    - in pipe, exit status depends on the status of the last cmd in pipe
        false | true -> sucess
        true | false -> failure
    - if single cmd, exit status depends on status of cmd
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
                - wildcard expansion:
                    - if nothing to expand to, orginal arg is passed in
                    - if its a path, the * must happen in the last part of the path.
                    - Matches to zero or more chrs that replace *
        int numOfArgs:
            - len of the args array, not including the null ender in the list of args
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
- test1.sh && testJobMaking.c:
    - holds a list of cmds to test mysh
    - see Test section
- librarys:
    - used arraylist.c from class 

Tests:
- Test Interactive Mode:
    In interactive mode, we want to check that mysh is parsing the typed
    in input correctly. So we are going to use the:
    testJobMaking.c:
        - Look at make seciton to see how to run it
        - this will test that we are expandins/parsing the bare names, input/output
        redirections, and wildcards properly. 
        - Input1: "echo man name >    txt.txt"
            - checking output redirection, bare name exapansion, blank space check
            - Expected Output:
                Job: 
                    Exec: /bin/echo
                    Args: |echo||man||name||(null)|
                    InRe: 
                    OuRe: txt.txt
        - Input2: "cat <  txt.txt     "
            - checking input redirection, bare name exapansion, blank space check
            - Expected Output:
                Job: 
                    Exec: /bin/cat
                    Args: |cat||(null)|
                    InRe: txt.txt
                    OuRe: 
        - Input3: "ls *.c"
            - checking bare name exapansion, wildcards
            - Expected Output:
                Job: 
                    Exec: /bin/ls
                    Args: |ls||sumFunction.c||builtins.c||mysh.c||arraylist.c||squareFunction.c||testJobMaking.c||job.c||(null)|
                    InRe: 
                    OuRe: 
        - Input4: "ls *.java >txt.txt"
            - checking redirection, bare name exapansion, wildcards
            - Expected Output:
                 Job: 
                    Exec: /bin/ls
                    Args: |ls||*.java||(null)|
                    InRe: 
                    OuRe: txt.txt
        - Input5: "ls testFolder2/*.txt >txt.txt"
            - checking redirection, bare name exapansion, wildcards in end of paths
            - Expected Output:
                Job: 
                    Exec: /bin/ls
                    Args: |ls||testFolder2/test1.txt||testFolder2/test2.txt||testFolder2/input.txt||(null)|
                    InRe: 
                    OuRe: txt.txt
        - Input6: "pwd"
            - checking builtins
            - Expected Output:
                Job: 
                    Exec: pwd
                    Args: |pwd||(null)|
                    InRe: 
                    OuRe: 

- Test Batch Mode: 
    WHEN RUNNING BATCH MODE, USE THE MAKE COMMAND to build and run the test1.sh
        this is bc the test1.sh also depends on our .c files that need to be compilied.

    The following outlines the details in the "test1.sh" file.
    - wildcards
        - using wildcards with built-ins
            - ex: ls *.c
            - ls testFolder2/te*.txt
        - using wildcards with other commands
            - ex: cat *.txt
        - no matching files found
            - return default output that normal shell would return
    - redirection
        - input redirection
            - ex: sumFunction < sumTest.txt
                - the sumFunction is a simple program written that adds up all the numbers in a file. We are passing in a file sumTest as input redirection
        - output redirection
            - using redirection with built-ins
                - ex: pwd > output.txt
        - using multiple redirections together
                - ex: ./sumFunction < sumTest.txt > sumOutput.txt
        
    - pipe
        - using pipes with the other functions and built-ins
        - using pipes with redirection
            - ex: ./sumFunction < sumTest.txt | ./squareFunction
    - builtins
        - cd
            - cd with no args
                - print out error
            - cd with 1 arg
            - cd with 2+ args
                - print out error
        - pwd
            - pwd with no args
            - pwd with 1+ args
                - print out error
        - which
            - called with no args
                - print out error (or print out nothing)
            - called with 1 arg 
            - called with 2+ args
    - conditinals
        - then
            - calling then/else after "which ls" (true)
                - testResults1.txt should display "test1 passed"
        - else
            - calling then/else after "pwd a" (false)
                - testResults2.txt should display "test2 failed"

    - bare names
        - bare names are properly expanded to meet the specific file path.
        - this is test by all the cmds above
