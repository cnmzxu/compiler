#Makefile for Compiler Programming Assignment
#author: xumz

#tools
CC = gcc
FLEX = flex
BISON = bison

CFLAGS = -std=c99 -I ./include/

#file sets
CFLIES = $(shell find ./code/ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILES = $(shell find ./code/ -name "*.l")
YFILES = $(shell find ./code/ -name "*.y")
LFC = $(shell find ./code/ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./code/ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

#targets
parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) -o parser $(filter-out $(LFO), $OBJS) -lfl -ly

syntax: lexical syntax-c
	$(CC) -c $(YFC) -o $(YFO)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

.PHONY: clean, test

test:
	./parser test.cmm

clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm-f $(LFC) $(YFC:.c=.h)
	rm -f *~
