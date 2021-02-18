all: sender rejestrator

rejestrator: rejestrator.c utils.c
	gcc -Wall -o rejestrator rejestrator.c utils.c -lrt

sender: sender.c
	gcc -Wall -o sender sender.c

clean:
	rm sender rejestrator