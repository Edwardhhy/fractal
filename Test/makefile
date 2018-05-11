all : test	
	./test

test : test.o ../libfractal/farctal.o ../libfractal/tools.o
	gcc -Wall -lcunit -o -test ../libfractal/farctal.o ../libfractal/tools.o

test.o : test.c
	gcc -Wall -c test.c -I$(HOME)/local/include
