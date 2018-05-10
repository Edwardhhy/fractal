all: main
	make clean

main: main.o 
	cd libfractal && make
	gcc -Wall -g -o main main.o libfractal/libfractal.a -lSDL -lpthread

main.o: main.c
	gcc -Wall -g -c main.c -lpthread

lib:
	cd libfractal && make
	make clean

clean:
	rm -rf *.o libfractal/*.o libfractal/*.a

.SILENT: 
