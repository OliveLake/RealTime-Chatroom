all: server.out client.out
server.out: server.o
	gcc -O3 -Wall -pthread -o server.out server.o
client.out: client.o 
	gcc -O3 -Wall -pthread -o client.out client.o 
server.o: server.c
	gcc -O3 -Wall -c server.c
client.o: client.c
	gcc -O3 -Wall -c client.c

.PHONY: clean
clean:
	rm -f *.o *.out
