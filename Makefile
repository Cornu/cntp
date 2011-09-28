CC = gcc
CFLAGS = -Wall -O2

cntp: cntp.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o *~ cntp
