CC = gcc
DEBUG = -D DEBUG=0
CFLAGS = -g -std=c99 -Wall -Werror -fsanitize=address,undefined $(DEBUG)

mysh: mysh.o job.o arraylist.o builtins.o
	$(CC) $(CFLAGS) -o $@ $^ 
testJobMaking: testJobMaking.o job.o arraylist.o builtins.o
	$(CC) $(CFLAGS) -o $@ $^
batchmodetest: clean mysh sumFunction squareFunction
	./mysh test1.sh 
sumFunction: sumFunction.o
	$(CC) $(CFLAGS) -o $@ $^ 

squareFunction: squareFunction.o
	$(CC) $(CFLAGS) -o $@ $^ 

sumFunction.o: sumFunction.c
	$(CC) $(CFLAGS) -c $^

squareFunction.o: squareFunction.c
	$(CC) $(CFLAGS) -c $^ 

*.o: *.h 

removeTXTFiles:
	rm -f -r directory.txt sumOutput.txt testResults.txt testResults2.txt 

clean: removeTXTFiles
	rm -f -r *.gch *.o *.dSYM mysh testJobMaking sumFunction squareFunction
