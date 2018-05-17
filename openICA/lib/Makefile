PACKAGE=libmathutil.a

SRC=mathtool.cc svdcmp.cc types.cc fft.cc
OBJ=$(SRC:.cc=.o)

#DEBUG=-D__DEBUG
POSIX=-D_XOPEN_SOURCE=600
### VERSION and BUILD ###

### Compiler option ###
CPP=g++
CPPFLAGS=-O2 -Wall $(POSIX) $(DEBUG) -I../include -I./include
LDFLAGS=-lm

### Rules ###
all: $(PACKAGE)

.cc.o :
	$(CPP) -c $(CPPFLAGS) $<
$(PACKAGE) : $(OBJ)
	ar crus $@ $(OBJ)

clean :
	rm -f *.o $(PACKAGE) core

rebuild: clean all
