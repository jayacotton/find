This is a find command for cp/m written for z88dk tool chain.

To build, copy the repo to your linux machine, (or possibly cygwin).
and type make.

Other make commands are, make just (indent code) make install (put code in /var/www/html) make clean (clear stuff out)
make scope  (set up cscope) make check (generate check sum).

You will likely need to edit the Makefile to correct the install path etc.

To enable tracing set -DDEBUG in the Makefile.

Usage:

```
C>find . -name *.c
C:FIND    .C  
C:CHARIO  .C  
C:HELLO   .C  
G:HELLO   .C  
G:FIND    .C  
H:RM      .C  
H:HELLO   .C  
H:TAIL    .C  
H:WILDEXP .C  
I:HELLO   .C  
I:RM      .C  
I:TAIL    .C  
I:WILDEXP .C  
```
Running find to collect all the file names on the machine, includeing network mounted drives (if they exist).
In this example, I trucated the list, I have quite a few files on my drives.
```
C>find .
A:CCP     .SPR
A:CPM3NET .HLP
A:CPM2NET .HLP
A:CPNET3  .HLP
A:CPNET12 .HLP
A:HELP    .HLP
B:ASM     .COM
................
J:RTCNTP  .COM
J:RUNMICRO.COM
J:XEQ     .COM
J:ENDLIST .COM
```
Here is a practical example of use.  Find all the file names, then count them.  There is a limit to the number of file names
that can be buffered for the log file.  Due to cp/m's pecueler (primitive) file system, one needs to be very carfull of when
and where to write log files, and scan directories for names.   I think a real top end is about 1200 to 1400 names.  After that
expect to run out of memory.
```
C>find . -o c:log.log

C>wc c:log.log
19072 3768 1192 C:LOG.LOG
```
The future of find is to expand it to exec a worker program to do a few tasks on the file names.  I.e.   find . -name *.c -exec jump 
around {} \;

Where -exec can run any program (that fits in ram) and feed it file names.


