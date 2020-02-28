TARGET  := sort
CFLAGS  := -Wall -pg
CC      := gcc

sort : line.o chunk.o data_file.o sort.o
	$(CC) $(CFLAGS) -o $@ $^

.o: .c 
	$(CC) $(CFLAGS) -c $^ 

clean :
	rm -f *.o $(TARGET)
