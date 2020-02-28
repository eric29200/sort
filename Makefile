TARGET  := sort
CFLAGS  := -Wall -O2
CC      := gcc

sort : mem.o line.o chunk.o data_file.o sort.o
	$(CC) $(CFLAGS) -o $@ $^

.o: .c 
	$(CC) $(CFLAGS) -c $^ 

clean :
	rm -f *.o $(TARGET)
