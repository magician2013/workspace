PACKAGE=bt_ica

SRC=bt_ica.cc fdica_batch.cc sigpro.cc infomax.cc vector_out.cc
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
	$(CPP) -o $@ $(OBJ) $(LDFLAGS) 

clean :
	rm -f *.o $(PACKAGE) core

rebuild: clean all