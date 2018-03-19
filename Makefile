CC = gcc
CFLAG = -g

hw2: hw2.o init.o reads.o execute.o
	$(CC) -o hw1 hw2.o init.o reads.o execute.o
.c.o:
	$(CC) $(CFLAG) -c $< 

clean:
	rm -f *.o

