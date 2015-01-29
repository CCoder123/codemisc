CFLAGS = -g
CLIBS = -lpthread  

CC = gcc
BIN = test 
SRC = $(wildcard *.c)

all:
	$(CC) $(SRC) $(CFLAGS) $(CLIBS) -o $(BIN)

clean:
	rm *.o $(BIN) -rf
