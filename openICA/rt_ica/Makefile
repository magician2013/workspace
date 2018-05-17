PACKAGE=rt_ica

SRC=rt_ica.cc fdica_blkwise.cc oss.cc sigpro.cc inkey.cc
OBJ=$(SRC:.cc=.o)

POSIX=-D_XOPEN_SOURCE=600
### VERSION and BUILD ###

### Compiler option ###
CPP=g++
CPPFLAGS=-O2 -Wall $(DEBUG) $(POSIX) -I../include -I../lib/include
LDFLAGS=-lm -lpthread -lmathutil -L../lib 

### Rules ###
all: $(PACKAGE)

.cc.o :
	$(CPP) -c $(CPPFLAGS) $<
$(PACKAGE) : $(OBJ)
	$(CPP) -o $@ $(OBJ) $(LDFLAGS) 

clean :
	rm -f *.o $(PACKAGE) core

rebuild: clean all