This is a find command for cp/m written for z88dk tool chain.

To build, copy the repo to your linux machine, (or possibly cygwin).
and type make.

Other make commands are, make just (indent code) make install (put code in /var/www/html) make clean (clear stuff out)
make scope  (set up cscope) make check (generate check sum).

You will likely need to edit the Makefile to correct the install path etc.

To enable tracing set -DDEBUG in the Makefile.

With the latest push, the command processing has been worked on extensively, and new commands have been added.
```
-d[rive] <drive letter>   The expected value is A->P, do not add the ':'.
-u[ser] <user number>     The expected value is a number between 0 and 15.
-a[lluser]                No expected value.  Set search for all user spaces.
-o[utput] <log file>      The expected value is a file name for the log.
-n[ame] <search string>   The expected value is a CP/M file name or wildcard.
.                         Search all drives, expects no value
-h[elp]                   Print a helpful message, expects no value.
```
I should point out that user does not permanently change the user number, unless find crashes
somehow.  So if find gets a gross error, you my need to say  c:user 0 or somesuch.

Find assumes user 0 unless told otherwise.  This can be an issue for say user 4.  A find to locate user 4
files started as user 4 will find files for user 0 unless you specify user 4 in your command.

Usage:

```
C>find . -name *.c
C0:FIND    .C  
C0:CHARIO  .C  
C0:HELLO   .C  
G0:HELLO   .C  
G0:FIND    .C  
H0:RM      .C  
H0:HELLO   .C  
H0:TAIL    .C  
H0:WILDEXP .C  
I0:HELLO   .C  
I0:RM      .C  
I0:TAIL    .C  
I0:WILDEXP .C  
```
Running find to collect all the file names on the machine, including network mounted drives (if they exist).
In this example, I truncated the list, I have quite a few files on my drives.
```
C>find .
A0:CCP     .SPR
A0:CPM3NET .HLP
A0:CPM2NET .HLP
A0:CPNET3  .HLP
A0:CPNET12 .HLP
A0:HELP    .HLP
B0:ASM     .COM
................
J0:RTCNTP  .COM
J0:RUNMICRO.COM
J0:XEQ     .COM
J0:ENDLIST .COM
```
Here is a practical example of use.  Find all the file names, then count them.  There is a limit to the number of file names
that can be buffered for the log file.  Due to cp/m's peculier (primitive) file system, one needs to be very careful of when
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


