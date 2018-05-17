YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

CC = gcc

CFLAGS = -lncurses -I$(ICUNIT) -I$(IDIR)

IDIR	= ../include/
SRCDIR 	= ../src/
EQMGRSRC = $(SRCDIR)equipmentMgr.c

LIB 	= ../lib/
ICUNIT 	= ../CUnit/CUnit/Headers/

LCUNIT 	= -L$(LIB) -lcunit
LCURL 	= -lcurl

TESTS 	= 	test_hello_world \
			test_get \
			test_post \
			test_linked_list_methods

all: $(TESTS)

$(TESTS): 
	$(CC) $@.c $(EQMGRSRC) $(CFLAGS) -o $@ $(LCUNIT) $(LCURL) 

runAll:
	for x in $(TESTS); do echo ${GREEN}"Running "$$x"..."${NC}; ./$$x; done

clean:
	find . -type f ! -iname "*.c" ! -iname "makefile" -delete