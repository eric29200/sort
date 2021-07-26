TARGET  := sort external_sort
CFLAGS  := -Wall -O2
CC      := gcc

sort: sort.o
	$(CC) $(CFLAGS) -o $@ $^

external_sort: external_sort.o
	$(CC) $(CFLAGS) -o $@ $^

.o: .c 
	$(CC) $(CFLAGS) -c $^ 

clean :
	rm -f *.o $(TARGET)
