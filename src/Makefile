CC=gcc
CXX=$(CC)
CFLAGS=-g -O3
LDFLAGS=-lm 

SRC_FILES=acc_timer.cpp linux_sys_util.cpp

LIB_FILENAME=libacc_timer.so
HEADER_FILENAME=acc_timer.h

PREFIX=/usr/local

default: build

build: so

so: $(SRC_FILES) $(HEADER_FILENAME)
	$(CXX) -shared $(CFLAGS) -o $(LIB_FILENAME) -fPIC $(SRC_FILES) $(LDFLAGS)

install: so $(HEADER_FILENAME)
	sudo cp $(LIB_FILENAME) $(PREFIX)/lib64
	sudo cp $(HEADER_FILENAME) $(PREFIX)/include

clean:
	rm $(LIB_FILENAME)
