CFLAGS=-fno-omit-frame-pointer -fPIC -Bdynamic -g -lprofiler
LDFLAGS= -L /usr/local/gperftools/lib -ltcmalloc_and_profiler -lpthread
CINCLUDE = -I /usr/local/gperftools/include

TARGETS = test

all:
	gcc test.c ../src/threadpool.c $(CINCLUDE) $(LDFLAGS) $(CFLAGS) -o $(TARGETS)
	gcc 1.c ../src/threadpool.c $(CINCLUDE) $(LDFLAGS) $(CFLAGS)

clean:
	rm -f $(TARGETS) *~ */*~ */*.o
