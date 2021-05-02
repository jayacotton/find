/*
Find for CP/M coded in 'c' z88dk.

find  [starting-point...] [expression]

	starting-point		"."  or "c:"  
		where  "."  is all drives
		and "c:" just look on this drive
		if omited works the same as "."

	expression 		"*.c" or "*.com"  etc.
			-name ....   (assumed...)

	-O [file name]  	create a file and list output to that

Remember that in CP/M land we have 64k of ram, and 1/4 of that is used
by CP/M itself.  So, we limit the number of things that the program can
do, in order to reduce the size.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trace.h"

char searchkey[13] =
    { '?', '?', '?', '?', '?', '?', '?', '?', '.', '?', '?', '?', 0 };

unsigned char drive_table[16] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

struct fcb Fcb;

char diskbuf[128];		// each directory entry has 4 names in it
char logfile[14];
int logflag = 0;
FILE *log;
int version;

void usage(){
printf("Find command by Jay Cotton, V1 4/28/2021\n");
printf("running on CP/M v%x\n",version);
printf("\nfind [drive] [flags] [options]\n");
printf("       drive = <letter>:  or . to match all drives\n");
printf("       flags = -O for log file\n");
printf("               -name [file name]\n");
printf("                      file name can be CP/M wildcard\n");
}
// figure out if drive is on the system
int existlocal(int drive)
{
    TRACE("existlocal");
    return 1;
}

// figure out if drive is mounted on the network
int existnet(int drive)
{
    TRACE("existnet");
    return 1;
}

// with cpm 3 we can ignore drive select errors
void seterrstat()
{
    TRACE("seterrstat");
// *INDENT-OFF* 
#asm
    ld c, $2d 
    ld e, 255 
    call 5 
#endasm
// *INDENT-ON*
} int getversion()
{
// 22 = cp/m 2.2
// 31 = cp/m 3.1
// else cp/net

    TRACE("getversion");
    version = bdos(CPM_VERS, 0);
#ifdef DEBUG
    printf("version %x\n", version);
#endif
    return version;
}

void initdrivetab(char *type)
{
    int i;
    int drive;

    TRACE("initdrivetab");
    drive = tolower(*type) - 'A';
    if (*type != '.')		// do all drives ?
    {
	// mark the drive table empty
	memset(drive_table, 0xff, 16);
	// range check the drive letter and set the table
	if ((drive >= 0) && (drive <= 16)) {
	    // determin if the drive is preset befor
	    // committing it to the list, also need to
	    // determin if the drive is a network drive,
	    // needs special handling....
	    if (existlocal(drive)) {
		drive_table[drive] = drive;
	    } else if (existnet(drive)) {
		drive_table[drive] = drive;
	    }
	}
    } else {
	for (i = 0; i < 16; i++) {
	    if (existlocal(i)) {
		drive_table[i] = i;
	    }
	}
    }

}

void setsearchkey(char *key)
{
    TRACE("setsearchkey");
    memset(searchkey, 0, 11);
    strcpy(searchkey, key);
}

void setlogname(char *name)
{
    TRACE("setlogname");
    strcpy(logfile, name);
    log = fopen(logfile, "w");
    logflag++;
}

// print file names from the directory buffer
int offset;
char disk;
char name[9];
char ext[4];
char pbuf[80];
void printnames(unsigned char drive, int index)
{
    TRACE("printnames");
    offset = index * 32;
    disk = drive + 'A';
    strncpy(name, &diskbuf[offset + 1], 8);
    name[8] = 0;
    strncpy(ext, &diskbuf[offset + 9], 3);
    ext[3] = 0;
    ext[0] &= 0x7f;
    if (logflag) {
	memset(pbuf, 0, 80);
	fprintf(log, "%c:%8s.%3s\r", disk, &name, &ext);
	fflush(log);		// don't trust cp/m
    } else {
	printf("%c:%8s.%3s\n", disk, &name, &ext);
    }
    SNAP(diskbuf, 128, 4);
#ifdef DEBUG1
    exit(1);
#endif
}

// set the dma address
void setdma()
{
    TRACE("setdma");
    bdos(CPM_SDMA, diskbuf);
}

// set up the fcb for file search
void initfcb(unsigned char drive)
{
    char *p;

    TRACE("initfcb");
    parsefcb(Fcb, searchkey);
    //Fcb.drive = drive;
    SNAP(Fcb, sizeof(struct fcb), 4);
}

// get the first directory buffer
// searchkey is the prototype file name to look for.
int searchfirst()
{
    int ret;
    TRACE("searchfirst");
    ret = bdos(CPM_FFST, Fcb);
    TVAL("ret = %d\n", ret);
    return ret;
}

// get the next directory buffer
int searchnext()
{
    int ret;
    TRACE("searchnext");
    ret = bdos(CPM_FNXT, 0);
    TVAL("ret = %d\n", ret);
    return ret;
}

// select the disk drive to search
int selectdrive(unsigned char drive)
{
    TRACE("selectdrive");
    bdos(CPM_LGIN, drive);
    return 1;
}

// process search on drive
void checkdrive(unsigned char drive)
{
    int i;
    TRACE("checkdrive");
    setdma();
    initfcb(drive);
    selectdrive(drive);
    if ((i = searchfirst()) != -1)
	printnames(drive, i);
	else
	return;
    while ((i = searchnext()) != -1) {
	printnames(drive, i);
    }
}

// get the work done here
void Process()
{
    int i;
    TRACE("Process");
#ifdef DEBUG
    printf("Looking for %s\n", searchkey);
#endif
    for (i = 0; i < 16; i++) {
	if (drive_table[i] <= 16) {
#ifdef DEBUG
	    printf("process %c:\n", drive_table[i] + 'A');
#endif
	    checkdrive(drive_table[i]);
	}
    }
}

void main(int argc, char *argv[])
{
    memset(logfile, 0, 14);

    version = getversion();
    if (version == 0x31)
	seterrstat();
    if (strstr(argv[1], "-H")) {
	usage();
	exit(1);
    }
    if ((argc >= 1) && (argc <= 5)) {
	initdrivetab(argv[1]);
	if (strstr(argv[2], "-NAME")) {
	    setsearchkey(argv[3]);
	} else if (strstr(argv[2], "-O")) {
	    setlogname(argv[3]);
	} else if (strstr(argv[3], "-O")) {
	    setlogname(argv[4]);
	}
	Process();
    }
    if (logflag)
	fclose(log);
}
