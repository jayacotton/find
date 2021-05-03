CFLAGS	= +cpm -Wall --list --c-code-in-asm -pragma-include:zpragma.inc
#CFLAGS	= +cpm -Wall -DDEBUG --list --c-code-in-asm -pragma-include:zpragma.inc
LINKOP	= +cpm -create-app -m  -pragma-include:zpragma.inc
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
	$(CP) FIND.COM $(DESTDIR)find.com

check:
	$(SUM) *.COM

