CFLAGS  := -Wall -O2
CC      := gcc

all: sort external_sort

sort: mem.o sort.o
	$(CC) $(CFLAGS) -o $@ $^

external_sort: mem.o external_sort.o
	$(CC) $(CFLAGS) -o $@ $^

.o: .c 
	$(CC) $(CFLAGS) -c $^ 

clean :
	rm -f *.o sort external_sort
