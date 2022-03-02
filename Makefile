#CFLAGS   = +cpm -Wall -D__CLASSIC -DOLD -pragma-include:zpragma.inc
#CFLAGS	= +cpm -Wall --list --c-code-in-asm -D__CLASSIC -DOLD -pragma-include:zpragma.inc
CFLAGS  = +cpm -compiler=sdcc -Wall -D__CLASSIC -DOLD --max-allocs-per-node200000 -pragma-include:zpragma.inc

LINKOP   = +cpm -create-app -m -pragma-include:zpragma.inc

CFLAGS85 = +cpm -clib=8085 -Wall -D__CLASSIC -DOLD -pragma-include:zpragma.inc
LINKOP85 = +cpm -clib=8085 -create-app -m -pragma-include:zpragma.inc

DESTDIR = ~/HostFileBdos/c/
DESTDIR1 = /var/www/html
SUM = sum
CP = cp
INDENT = indent -kr -ut
SUDO = sudo

# define SNAP to null when debugging is done.
SNAP =
#SNAP =	snaplib.o

all: find

find: find.o snaplib.o mygetopt.o
	zcc $(LINKOP) -ofind find.o mygetopt.o $(SNAP)

mygetopt.o: mygetopt.c
	zcc $(CFLAGS) -c mygetopt.c

find.o: find.c
	date > date.h
	zcc $(CFLAGS) -c find.c

snaplib.o: snaplib.c
	zcc $(CFLAGS) -c snaplib.c

find85: find85.o mygetopt85.o
	zcc $(LINKOP85) -ofind85 find85.o mygetopt85.o

mygetopt85.o: mygetopt.c
	zcc $(CFLAGS85) -o mygetopt85.o -c mygetopt.c

find85.o: find.c
	date > date.h
	zcc $(CFLAGS85) -o find85.o -c find.c

clean:
	$(RM) *.o *.err *.lis *.def *.lst *.sym *.exe *.COM  *.map find find85

justify:
	$(INDENT) find.c
	$(INDENT) mygetopt.c
	$(INDENT) snaplib.c

scope:
	cscope

install:
	$(SUDO) $(CP) ./*.COM $(DESTDIR1)/. 
	$(CP) FIND.COM $(DESTDIR)find.com

check:
	$(SUM) *.COM

