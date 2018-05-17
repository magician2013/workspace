PACKAGE=mk_nbf

SRC=mk_nbf.cc nbf.cc
OBJ=$(SRC:.cc=.o)


### VERSION and BUILD ###

### Compiler option ###
CPP=g++
CPPFLAGS=-O2 -Wall $(DEBUG) -I../include -I../lib/include
LDFLAGS=-lm -lmathutil -L../lib

### Rules ###
all: $(PACKAGE)

.cc.o :
	$(CPP) -c $(CPPFLAGS) $<
$(PACKAGE) : $(OBJ)
	$(CPP) -o $@ $(OBJ)  $(LDFLAGS)

clean :
	rm -f *.o $(PACKAGE) core

rebuild: clean all