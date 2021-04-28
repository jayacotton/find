#CFLAGS	= +cpm -Wall -pragma-include:zpragma.inc
#LINKOP	= +cpm -create-app -pragma-include:zpragma.inc -DAMALLOC2 
CFLAGS	= +cpm -Wall --list --c-code-in-asm
#CFLAGS	= +cpm -Wall -DDEBUG --list --c-code-in-asm
LINKOP	= +cpm -create-app -m  
DESTDIR = ~/HostFileBdos/c/
DESTDIR1 = /var/www/html
SUM = sum
CP = cp
INDENT = indent -kr
SUDO = sudo

all: find

find: find.o snaplib.o
	zcc $(LINKOP) -ofind find.o snaplib.o

find.o: find.c
	zcc $(CFLAGS) -c find.c

snaplib.o: snaplib.c
	zcc $(CFLAGS) -c snaplib.c

clean:
	$(RM) *.o *.err *.lis *.def *.lst *.sym *.exe *.COM  *.map find

just:
	$(INDENT) find.c

scope:
	cscope

install:
	$(SUDO) $(CP) ./*.COM $(DESTDIR1)/. 

check:
	$(SUM) *.COM

