#Makefile for Compiler Programming Assignment
#author: xumz

#tools
CC = gcc
FLEX = flex
BISON = bison

CFLAGS = -std=c99 -I ./include/

#file sets
CFILES = $(shell find ./code/ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find ./code/ -name "*.l")
YFILE = $(shell find ./code/ -name "*.y")
LFC = $(shell find ./code/ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./code/ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)
COBJS = $(filter-out $(LFO), $(OBJS))
#targets
parser: syntax $(COBJS)
	$(CC) -o parser $(COBJS) -lfl -ly

syntax: lexical syntax-c
	$(CC) -c $(YFC) -o $(YFO)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -t -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

.PHONY: clean, test, git

test:
	./parser test.cmm

clean:
	rm -f parser 
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h) $(YFILE:.y=.output)
	rm -f *~

git: clean
	git add -A
	git commit -a
	git push
