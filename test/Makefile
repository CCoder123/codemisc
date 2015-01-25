CFLAG = -g -lpthread

all:
	gcc test.c ../hashtable/src/hashtable.c ../threadpool/src/threadpool.c $(CFLAG) -o test

clean:
	rm -rf test
