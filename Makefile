CC := gcc

CFLAGS := -Wall -Wextra -std=gnu99 -O3 -Wformat-truncation=0


tiny: main.c logger.o core.o socket.o csapp.o
	$(CC) $(CFLAGS) main.c logger.o core.o socket.o csapp.o -o tiny

run: tiny
	./tiny 10339

logger.o : logger.c logger.h
	$(CC) $(CFLAGS) -DENABLE_LOG -c logger.c -o logger.o

core.o: core.c core.h
	$(CC) $(CFLAGS) -c core.c -o core.o

socket.o: socket.c socket.h
	$(CC) $(CFLAGS) -c socket.c -o socket.o

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c -o csapp.o

clean:
	rm -f *.o tiny