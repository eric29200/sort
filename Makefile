CFLAGS  := -Wall -O2 -g
CC      := gcc

all: sort external_sort

sort: mem.o line.o chunk.o data_file.o sort.o
	$(CC) $(CFLAGS) -o $@ $^

external_sort: mem.o line.o chunk.o data_file.o external_sort.o
	$(CC) $(CFLAGS) -o $@ $^

.o: .c 
	$(CC) $(CFLAGS) -c $^ 

clean :
	rm -f *.o sort external_sort