TARGET  := sort
CFLAGS  := -Wall -O2
CC      := gcc

sort : mem.o sort.o main.o
	$(CC) $(CFLAGS) -o $@ $^

.o: .c 
	$(CC) $(CFLAGS) -c $^ 

clean :
	rm -f *.o $(TARGET)
