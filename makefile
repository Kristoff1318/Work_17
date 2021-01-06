all: client.c server.c
	gcc -o server.out server.c
	gcc -o client.out client.c

clean:
	rm *.out
