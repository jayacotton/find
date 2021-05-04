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

extern int selectdrive(unsigned char);

char searchkey[13] =
    { '?', '?', '?', '?', '?', '?', '?', '?', '.', '?', '?', '?', 0 };

unsigned char drive_table[16] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

typedef struct config_tbl {
    char stat;
    char id;
    int disk[16];
    int con;
    int list;
    int index;
    int header[4];
    char listdn;
    char buf[172 - 45];
} CONFIGTBL;

struct fcb Fcb;

typedef struct name_entry {
    char drive;
    char col;
    char name[8];
    char dot;
    char ext[3];
    char end;
    struct name_entry *next;
} NAMEENTRY;

char diskbuf[128];		// each directory entry has 4 names in it
char logfile[14];
int logflag = 0;
NAMEENTRY *logp;
int logcount = 0;
FILE *log;
int version;
char ldrive;
int lres;
int nflag = 0;

void usage()
{
    printf("Find command by Jay Cotton, V1 4/28/2021\n");
    printf("running on CP/M v%x\n", version);
    printf("\nfind [drive] [flags] [options]\n");
    printf("       drive = <letter>:  or . to match all drives\n");
    printf("       flags = -O for log file\n");
    printf("               -name [file name]\n");
    printf("                      file name can be CP/M wildcard\n");
}

// figure out if drive is on the system
// the methode below only works on cp/m 2.2
// systems.
// on cp/m 3 systems we just ignore the drive select
// fault.

int existlocal(int drive)
{
    TRACE("existlocal");
    lres = 0;
    ldrive = (char) drive;
// for cpm2.2 we have to do the select and dodge the error
// checking that is built in to select drive
    if ((version & 0xff) == 0x22) {
// *INDENT-OFF*
#asm
	ld	hl,(1)
	ld	a,l
	and	a,0f0h
	add	a,3*9	; get address of magic drive select
	ld	l,a
	ld	e,0
	ld	a,(_ldrive)
	ld	c,a
; this is a difficult piece of code because there is no call (hl) instruction
	ld	(_addr),hl
	db	0cdh
._addr	dw	0
	ld	(_lres),hl
#endasm
// *INDENT-ON*
    } else {
// for cpm3 and cp/net we just try to select the drive
// if it returns 0xff we have a problem else all is o.k.
	if (selectdrive(drive) == 0)
	    lres = 0;
	else
	    lres = 1;
    }
#ifdef DEBUG
    printf("existlocal %d\n", lres);
#endif
    return lres;
}

// figure out if drive is mounted on the network
int existnet(int drive)
{
    CONFIGTBL *pp;

    TRACE(" existnet ");
    lres = 0;			// leave this here 
// cp/net always set the upper byte of version to 0x200
    if ((version & 0xf00) == 0x200) {
// now look for the drive list from cp/net
// *INDENT-OFF*
#asm
	ld	c,045h
	call	5
	ld	(_lres),hl
#endasm
// *INDENT-ON*
// lres is a pointer to the cp/net drive table
	pp = (CONFIGTBL *) lres;
	lres = 0;
#ifdef DEBUG
	if (pp->disk[drive])
	    printf("%d is network drive\n", drive);
	else
	    printf("%d is local drive\n", drive);
#endif
	if (pp->disk[drive]){
	    lres = 1;
	nflag++;
	}
    }
    return lres;
}

// with cpm 3 we can ignore drive select errors
void seterrstat()
{
    TRACE(" seterrstat ");
// *INDENT-OFF* 
#asm
    ld c, $2d 
    ld e, 255 
    call 5 
#endasm
// *INDENT-ON*
    return;
}

int getversion()
{
// 22 = cp/m 2.2
// 31 = cp/m 3.1
// else cp/net

    TRACE(" getversion ");
//*INDENT-OFF*
#asm
	ld	c,12
	call	5
	ld	(_version),hl
#endasm
//*INDENT-ON*
    return version;
}

void initdrivetab(char *type)
{
    int i;
    int drive;

    TRACE(" initdrivetab ");
    drive = tolower(*type) - 'a';
    // mark the drive table empty
    memset(drive_table, 0xff, 16);
    if (*type != '.')		// do all drives ?
    {
	// range check the drive letter and set the table
	if ((drive >= 0) & (drive <= 16)) {
	    // determin if the drive is preset befor
	    if (existlocal(drive)) {
		drive_table[drive] = drive;
	    }
	    if (existnet(drive)) {
		drive_table[drive] = drive;
	    }
	}
    } else {
	// do all 16 possible drives
	for (i = 0; i < 16; i++) {
	    if (existlocal(i)) {
		drive_table[i] = i;
	    } else if ((version == 0x231) && (nflag == 0))
		drive_table[i] = i;
	    if (existnet(i)) {
		drive_table[i] = i;
	    }
	}
    }
#ifdef DEBUG3
    for (i = 0; i < 16; i++) {
	printf("drive %c %x\n", i + 'A', drive_table[i]);
    }
#endif
}

void setsearchkey(char *key)
{
    TRACE(" setsearchkey ");
    memset(searchkey, 0, 11);
    strcpy(searchkey, key);
}

void setlogname(char *name)
{
    TRACE(" setlogname ");
    strcpy(logfile, name);
    logflag++;
}

// print file names from the directory buffer
int offset;
char disk;
char name[9];
char ext[4];
char pbuf[80];
NAMEENTRY *pname;
void printnames(unsigned char drive, int index)
{
    TRACE(" printnames ");
    offset = index * 32;
    disk = drive + 'A';
    strncpy(name, &diskbuf[offset + 1], 8);
    name[8] = 0;
    strncpy(ext, &diskbuf[offset + 9], 3);
    ext[3] = 0;
    ext[0] &= 0x7f;
    if (logflag) {
	if (logp == NULL) {
	    logp = (NAMEENTRY *) calloc(1, sizeof(NAMEENTRY));
	    if (logp == 0) {
		printf("Out of memory\n");
		exit(1);
	    }
	    pname = logp;
	    sprintf(pname, "%c:%8s.%3s\0 ", disk, &name, &ext);
	    logcount++;
	    return;
	}
	pname->next = (NAMEENTRY *) calloc(1, sizeof(NAMEENTRY));
	if (pname->next == 0) {
	    printf("Out of memory\n");
	    exit(1);
	}
	pname = pname->next;
	sprintf(pname, "%c:%8s.%3s\0", disk, &name, &ext);
	logcount++;
	return;

    } else {
	printf("%c:%8s.%3s\n", disk, &name, &ext);
    }
    //SNAP(diskbuf, 128, 4);
#ifdef DEBUG1
    exit(1);
#endif
}

// set the dma address
void setdma()
{
    TRACE(" setdma ");
    bdos(CPM_SDMA, diskbuf);
}

// set up the fcb for file search
void initfcb(unsigned char drive)
{
    char *p;
    TRACE(" initfcb ");
    parsefcb(Fcb, searchkey);
    //SNAP(Fcb, sizeof(struct fcb), 4);
}

// get the first directory buffer
// searchkey is the prototype file name to look for.
int searchfirst()
{
    int ret;
    TRACE(" searchfirst ");
    ret = bdos(CPM_FFST, Fcb);
    TVAL("ret = %d\n", ret);
    return ret;
}

// get the next directory buffer
int searchnext()
{
    int ret;
    TRACE(" searchnext ");
    ret = bdos(CPM_FNXT, 0);
    TVAL("ret=%d\n", ret);
    return ret;
}

// select the disk drive to search
int selectdrive(unsigned char drive)
{
    TRACE(" selectdrive ");
    ldrive = drive;
// cpm3 can return 0 or ff
//    lres = bdos(CPM_LGIN, drive);
//*INDENT-OFF*
#asm
	ld	c,14
	ld	a,(_ldrive)
	ld	e,a
	call	5
	ld	hl,0
	ld	l,a
	ld	(_lres),hl
#endasm
//*INDENT-ON*
#ifdef DEBUG
    printf("CPM_LGIN returns %d\n", lres);
#endif
    if (lres == 0)
	return 1;
    return 0;
}

// process search on drive
void checkdrive(unsigned char drive)
{
    int i;
    TRACE(" checkdrive ");
    if (drive_table[drive] >= 16)
	return;
    setdma();
    initfcb(drive);
// selectdrive is a problem, its not working like the 
// documentation says it should.
    if (version != 0x231) {
	if (!selectdrive(drive))
	    return;
    } else
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
    TRACE(" Process ");
#ifdef DEBUG
    printf("Looking for %s\n", searchkey);
#endif
    for (i = 0; i < 16; i++) {
	if (drive_table[i] <= 16) {
#ifdef DEBUG
	    printf(" process % c:\n ", drive_table[i] + 'A');
#endif
	    checkdrive(drive_table[i]);
	}
    }
}

void main(int argc, char *argv[])
{
    int i;
    memset(logfile, 0, 14);
    logp = NULL;
    version = getversion();
    if ((version & 0xff) == 0x31)
	seterrstat();
#ifdef DEBUG
    printf("argv[1] = %s\n", argv[1]);
#endif
    if (strstr(argv[1], "-H")) {
	usage();
	exit(1);
    }
    if (argc >= 1) {
#ifdef DEBUG2
	initdrivetab(".");
#else
	initdrivetab(argv[1]);
#endif
	if (strstr(argv[2], "-NAME"))
	    setsearchkey(argv[3]);
	if (strstr(argv[4], "-NAME"))
	    setsearchkey(argv[5]);
	if (strstr(argv[2], "-O"))
	    setlogname(argv[3]);
	if (strstr(argv[4], "-O"))
	    setlogname(argv[5]);

	Process();
    }
    if (logflag) {
// delay file open to here, cp/m can't handle changing drive allocations
// during file scaning
	remove(logfile);
	log = fopen(logfile, "w");
	if (log == 0) {
	    printf("can't make %s\n", logfile);
	    exit(1);
	}
	for (pname = logp; pname = pname->next; pname->next) {
	    //SNAP(pname, 32, 4);
	    fprintf(log, "%s\r", pname);
	}
	fclose(log);
    }
}
