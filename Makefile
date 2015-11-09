# Author: 		Ran Bao
# Date:			16/Sept/2015
# Description:	Light weighted http server

# Definitions for compiler and other flags
CC = cc
CFLAGS = -Os -Wall -Wextra -Wstrict-prototypes -Wno-unused-parameter -Wno-unused-variable -g -std=c99 -D_GNU_SOURCE
INCLUDE = -I./include -I./src
DEL = rm

# Conditional flags
ifeq ($(CC),clang)
LD_LIBS = -lpthread
else
LD_LIBS = -pthread
endif

# Definitions for object
PROG_OBJ = src/http_parser.o src/webhttpd.o src/config.o src/parser.o src/datetime.o src/client.o src/cgi.o src/response.o src/worker.o

# Targets
default: src/webhttpd.out
test: test/parser.out test/cgi.out test/datetime.out test/cgi.out test/site_package.out test/http.out test/exec.out
all: default test


# Compile Source
src/%.o: src/%.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

# Compile Testcases
test/%.o: test/%.c $(DEPS)
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

# Link
src/webhttpd.out: $(PROG_OBJ)
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/parser.out: test/parser.o src/http_parser.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/cgi.out: test/cgi.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/datetime.out: test/datetime.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/site_package.out: test/site_package.o src/config.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/http.out: test/http.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/exec.out: test/exec.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@


# Others
.PHONY: clean
clean:
	-$(DEL) src/*.o src/*.out test/*.o test/*.out
