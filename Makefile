CC = gcc
DEBUG = -D DEBUG=0
CFLAGS = -g -std=c99 -Wall -fsanitize=address,undefined $(DEBUG)
mysh: mysh.o job.o arraylist.o builtins.o
	$(CC) $(CFLAGS) -o $@ $^
testJobMaking: testJobMaking.o job.o arraylist.o builtins.o
	$(CC) $(CFLAGS) -o $@ $^
*.o: *.h 

clean:
	rm -f -r *.o *.dSYM mysh
