CC = gcc
DEBUG = -D DEBUG=0
CFLAGS = -g -std=c99 -Wall -fsanitize=address,undefined $(DEBUG)
mysh: mysh.o job.o arraylist.o
	$(CC) $(CFLAGS) -o $@ $^
testJobMaking: testJobMaking.o job.o
	$(CC) $(CFLAGS) -o $@ $^
job.o: job.h 
mysh.o: mysh.c

clean:
	rm -f -r *.o *.dSYM words
