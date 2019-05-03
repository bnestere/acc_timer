CC=gcc
CXX=$(CC)
FC=gfortran
CFLAGS=-g -O3
CXXFLAGS=-std=c++11
LDFLAGS=-lm 

SRC_FILES=acc_timer.cpp linux_sys_util.cpp

FT_BINDING=acc_fortran_bindings.f90
FT_BINDING_LIB=libacc_ft.so
FT_MOD_FILE=acc_timer_m.mod

LIB_FILENAME=libacc_timer.so
HEADER_FILENAME=acc_timer.h

PREFIX=/usr/local
LIB_PREFIX=lib

default: build

build: so

so: $(SRC_FILES) $(HEADER_FILENAME)
	$(CXX) -shared $(CFLAGS) $(CXXFLAGS) -o $(LIB_FILENAME) -fPIC $(SRC_FILES) $(LDFLAGS)

ftbindings: $(FT_BINDING)
	$(FC) -shared -fPIC $(FT_BINDING) -o $(FT_BINDING_LIB)

install: so $(HEADER_FILENAME)
	sudo cp $(LIB_FILENAME) $(PREFIX)/$(LIB_PREFIX)
	sudo cp $(HEADER_FILENAME) $(PREFIX)/include

ftinstall: install ftbindings
	sudo cp $(FT_BINDING_LIB) $(PREFIX)/$(LIB_PREFIX)
	sudo cp $(FT_MOD_FILE) $(PREFIX)/$(LIB_PREFIX)

clean:
	rm $(LIB_FILENAME)
