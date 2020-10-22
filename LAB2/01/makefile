all: function.o main.c
	gcc -g -o main function.o main.c

function.o: function.c
	gcc -c -g -o function.o function.c

clean:
	rm -rf function.o