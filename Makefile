TARGET  := external_sort
CFLAGS  := -Wall -O2
CC      := gcc

external_sort: external_sort.o
	$(CC) $(CFLAGS) -o $@ $^

.o: .c 
	$(CC) $(CFLAGS) -c $^ 

clean :
	rm -f *.o $(TARGET)
