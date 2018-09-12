CC=gcc
CXX=g++
CFLAGS=-g
LDFLAGS=-lm -lacc_timer -pthread -lstdc++

simple: simple.c
	$(CC) $(CFLAGS) simple.c -o xsimple $(LDFLAGS)

multithread_multistage: multithread_multistage.cpp
	$(CXX) $(CFLAGS) multithread_multistage.cpp -o xmultithread_multistage $(LDFLAGS)

clean:
	rm xsimple xmultithread_multistage
