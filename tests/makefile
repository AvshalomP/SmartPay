YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

CC = gcc

CFLAGS = -lncurses -I$(ICUNIT) 

LIB 	= ../lib/
ICUNIT 	= ../CUnit/CUnit/Headers/

LCUNIT 	= -L$(LIB) -lcunit
LMHTTP 	= -L$(LIB) -lmicrohttpd
LCURL 	= -lcurl

TESTS 	= 	test_hello_world \
			test_get \
			test_post

all: $(TESTS)

$(TESTS): 
	$(CC) $@.c $(CFLAGS) -o $@ $(LCUNIT) $(LCURL) 

runAll:
	for x in $(TESTS); do echo ${GREEN}"Running "$$x"..."${NC}; ./$$x; done

clean:
	find . -type f ! -iname "*.c" ! -iname "makefile" -delete