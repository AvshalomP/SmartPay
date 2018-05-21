CC = gcc
CFLAGS = -lncurses -I$(IDIR) -I$(ICUNIT) -I$(IMHTTP)

IDIR	= ./include/
SRCDIR 	= ./src/
LIB 	= ./lib/
ICUNIT 	= ./CUnit/CUnit/Headers/
IMHTTP 	= ./libmicrohttpd/src/include/

LCUNIT 	= -L$(LIB) -lcunit
LMHTTP 	= -L$(LIB) -lmicrohttpd
LPTHREAD= -lpthread

SOURCES = $(SRCDIR)*.c

all: runHttpServer

runHttpServer:
	$(CC) $(SOURCES) $(CFLAGS) -o $@ $(LCUNIT) $(LMHTTP) $(LPTHREAD)

clean:
	rm runHttpServer