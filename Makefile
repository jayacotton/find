CFLAGS	= +cpm -Wall -pragma-include:zpragma.inc
#CFLAGS	= +cpm -Wall -DDEBUG --list --c-code-in-asm -pragma-include:zpragma.inc
LINKOP	= +cpm -create-app -m  -pragma-include:zpragma.inc
DESTDIR = ~/HostFileBdos/c/
DESTDIR1 = /var/www/html
SUM = sum
CP = cp
INDENT = indent -kr
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
	zcc $(CFLAGS) -c find.c

snaplib.o: snaplib.c
	zcc $(CFLAGS) -c snaplib.c

clean:
	$(RM) *.o *.err *.lis *.def *.lst *.sym *.exe *.COM  *.map find

just:
	$(INDENT) find.c
	$(INDENT) mygetopt.c

scope:
	cscope

install:
	$(SUDO) $(CP) ./*.COM $(DESTDIR1)/. 
	$(CP) FIND.COM $(DESTDIR)find.com

check:
	$(SUM) *.COM

