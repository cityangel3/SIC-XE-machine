CC = gcc
CFLAGS = -W -Wall 
TARGET = 20171700.out
OBJS = 20171700.o dump.o hash.o assemble.o linkload.o
SRCS = 20171700.h 20171700.c dump.c hash.c assemble.c linkload.c 

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
$(OBJS): 20171700.h 20171700.c dump.c hash.c assemble.c linkload.c
	$(CC) $(CFLAGS) -c 20171700.c dump.c hash.c assemble.c linkload.c

clean:
	rm -rf *. *.o *.lst *.out

