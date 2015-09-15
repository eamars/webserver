# Author: 		Ran Bao
# Date:			14/Sept/2015
# Description:	Light weighted http server

# Definition for compiler and other flags
CC = clang
CFLAGS = -Os -Wall -Wextra -Wstrict-prototypes -Wno-unused-parameter -g -std=c99 -D_GNU_SOURCE
INCLUDE = -Iinclude -Isrc
LD_LIBS = lib/http_parser.o
DEL = rm

# Targets
all: src/webhttpd.out
parser: test/parser.out
test: test/test.out
cgi: test/cgi.out
datetime: test/datetime.out

# Compile Source
src/main.o: src/main.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

src/config.o: src/config.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

src/parser.o: src/parser.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

src/datetime.o: src/datetime.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

# Compile Testcases
test/parser.o: test/parser.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

test/test.o: test/test.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

test/cgi.o: test/cgi.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

test/datetime.o: test/datetime.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@

# Link
src/webhttpd.out: src/main.o src/config.o src/parser.o src/datetime.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/parser.out: test/parser.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/test.out: test/test.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/cgi.out: test/cgi.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

test/datetime.out: test/datetime.o
	$(CC) $(CFLAGS) $(LD_LIBS) $^ -o $@

# Others
.PHONY: clean
clean:
	-$(DEL) src/*.o src/*.out test/*.o test/*.out
