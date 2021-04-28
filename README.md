This is a find command for cp/m written for z88dk tool chain.

To build, copy the repo to your linux machine, (or possibly cygwin).
and type make.

Other make commands are, make just (indent code) make install (put code in /var/www/html) make clean (clear stuff out)
make scope  (set up cscope) make check (generate check sum).

You will likely need to edit the Makefile to correct the install path etc.

To enable tracing set -DDEBUG in the Makefile.

Find will read file names from all the drives (.) or just one (x:) and match to a search string (see cp/m file names)
and produce a listing output.  The search string is indicated with the key -name [key] and the log file is indicated with
-o [log file name].

Note: expect to add a drive letter to the log file name, or find will try to place the log on the a: drive.

This command is handy on systems with a large amount of file names, especially on machines with many drives.

Find is a notion I got from using linux and coded from scratch with a UI in mind.  The original code was written in 
macro assembly.  With this new version running in C I may revisit the macro code and update it to a run alike version.

Find will report a system error when it runs off the end of the committed drive list.  On CP/M 3 this should not be an 
issue.  I am working on a scheam to get the allocated drive list from cp/m and use that to control the search path.

The future of find is to expand it to exec a worker program to do a few tasks on the file names.  I.e.   find . -name *.c -exec jump 
around {} \;

Where -exec can run any program (that fits in ram) and feed it file names.


