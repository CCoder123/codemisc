#CFLAGS = -D_REENTRANT -Wall -pedantic -Isrc
CFLAGS =  
LDLIBS = -lpthread

ifdef DEBUG
CFLAGS += -g
LDFLAGS += -g
endif

TARGETS = test/thrdtest test/heavy test/shutdown test/test

all: $(TARGETS)

src/threadpool.o: src/threadpool.c src/threadpool.h

test/shutdown: test/shutdown.o src/threadpool.o
test/test: test/test.o src/threadpool.o

test/thrdtest: test/thrdtest.o src/threadpool.o
test/thrdtest.o: test/thrdtest.c src/threadpool.h

test/heavy: test/heavy.o src/threadpool.o
test/heavy.o: test/heavy.c src/threadpool.h

clean:
	rm -f $(TARGETS) *~ */*~ */*.o
