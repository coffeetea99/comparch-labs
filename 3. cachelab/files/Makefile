%.o: %.c
	$(CC) -O2 -Wall -c -o $@ $<

all: cache.o cachesim.o
	$(CC) -Wall -o cachesim $^

clean:
	rm -f *.o cachesim

