CC=gcc
CXX=g++
CFLAGS=-g
LDFLAGS=-lm -lacc_timer -pthread

simple: simple.c
	$(CC) $(CFLAGS) simple.c -o xsimple $(LDFLAGS)

multithread_multistage: multithread_multistage.c
	$(CXX) $(CFLAGS) multithread_multistage.c -o xmultithread_multistage $(LDFLAGS)

clean:
	rm xsimple xmultithread_multistage
