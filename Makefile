CC = gcc
CFLAGS = -g -lcurses
TARGETS = vsh vsh.o builtin.o

vsh: vsh.o builtin.o
	$(CC) $(CFLAGS) -o vsh vsh.o builtin.o

vsh.o:
	$(CC) $(CFLAGS) -c main.c -o vsh.o

builtin.o:
	$(CC) $(CFLAGS) -c builtin.c -o builtin.o

clean:
	rm -f ${TARGETS}
