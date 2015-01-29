CFLAGS = -g 
LDLIBS = -lpthread

TARGETS = threadpool.c test.c 

all: 
	gcc -o test $(TARGETS) $(CFLAGS) $(LDLIBS)


clean:
	rm -rf test 
