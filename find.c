/* Find for CP/M coded in 'c' z88dk.

find  [starting-point...] [expression]

    starting-point        "."  or "c:"  
        where  "."  is all drives
        and "c:" just look on this drive
        if omited works the same as "."

    expression         "*.c" or "*.com"  etc.
            -name ....   (assumed...)

    -O [file name]      create a file and list output to that

Remember that in CP/M land we have 64k of ram, and 1/4 of that is used
by CP/M itself.  So, we limit the number of things that the program can
do, in order to reduce the size.

*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trace.h"

extern int selectdrive(unsigned char);

// default search key, matches all file names
unsigned char searchkey[13] =
    { '?', '?', '?', '?', '?', '?', '?', '?', '.', '?', '?', '?', 0 };

// The local drive table.  ff = nothing there
unsigned char drive_table[16] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

typedef struct config_tbl {
    char stat;
    char id;
    int disk[16];		// The list of network mounted drives
    int con;
    int list;
    int index;
    int header[4];
    char listdn;
    char buf[172 - 45];
} CONFIGTBL;

struct fcb Fcb;

typedef struct name_entry {
    char drive;			// drive letter
    char usr;			// user number
    char col;			// a :
    char name[8];		// the name
    char dot;			// a .
    char ext[3];		// the extension
    char end;			// a zero
    struct name_entry *next;	// pointer to the next one of these
} NAMEENTRY;

char diskbuf[128];		// each directory entry has 4 names in it
char logfile[14];		// name of log file
int logflag = 0;		// logging is requested
NAMEENTRY *logp;		// Point to log list root
int logcount = 0;		// A count of log entries
FILE *log;			// the Log file
int version;			// OPsys version number
char ldrive;			// general drive passin
int lres;			// general res passback
int nflag = 0;			// gets set after first network drive
int user = 0;			// user number
int olduser;			// save the old user number
int alluser = 0;		// search all user numbers
char *fstr = "Find command by Jay Cotton, V1";
char biospd = 9;
char areg = 0;
int bcreg = 0;
int dereg = 0;
int hlreg = 0;
int offset;			// FCB offset in the disk record
char disk;			// disk # always zero
// working buffers
char name[9];			// file name
char ext[4];			// file extension
char pbuf[80];			// working buffer
NAMEENTRY *pname;		// temporary pointer 


void usage()
{
    printf("%s %s, %s\n", fstr, __DATE__, __TIME__);
    printf("running on CP/M v%x\n", version);
    printf("\nfind <drive> [flags] [options]\n");
    printf("       -drive <d> or just '.' for all drives\n");
    printf("       -name <name string> uses CP/M wildcard\n");
    printf("       -user <number> search in a user space\n");
    printf("       -alluser search all user spaces\n");
    printf("       -output <log file>\n");
    printf("       -help print this output\n");
}

// figure out if drive is on the system
// the method below only works on cp/m 2.2
// systems.
// on cp/m 3 systems we just ignore the drive select
// fault.

int lbios(char drive)
{
    biospd = 9;
    bcreg = 14;
    hlreg = 0;
    dereg = drive;

    do {
//*INDENT-OFF*
#ifdef __SCCZ80
#asm
    ld    c,50
    ld    de,_biospd
    jp    5
    ; hl is set with return
#endasm
#endif
#ifdef __SDCC
__asm
    ld      c,50
    ld      de,_biospd
    jp      5
    ; hl is set with return
__endasm;
#endif

//*INDENT-ON*
    } while (0);
    return;
}

int existlocal(int drive)
{
    TRACE("existlocal");
    lres = 0;
    ldrive = (char) drive;
// for cpm2.2 we have to do the select and dodge the error
// checking that is built in to select drive
    if ((version & 0xff) == 0x22) {
	do {
// *INDENT-OFF*
#ifdef __SCCZ80
#asm
    ld      hl,(1)
    ld      a,l
    and     a,0f0h
    add     a,3*9       ; get address of bios seldsk
    ld      l,a
    ld      e,0
    ld      a,(_ldrive)
    ld      c,a
; use jp (hl) because there is no call (hl) instruction
    EXTERN  l_jphl      ; intrinsic function for sccz80
    call    l_jphl
    ld      (_lres),hl
#endasm
#endif
#ifdef __SDCC
__asm
    ld      hl,(1)
    ld      a,l
    and     a,0f0h
    add     a,3*9       ; get address of bios seldsk
    ld      l,a
    ld      e,0
    ld      a,(_ldrive)
    ld      c,a
    ; use jp (hl) because there is no call (hl) instruction
    EXTERN  l_jphl      ; external function for sdcc
    call    l_jphl
    ld      (_lres),hl
__endasm;
#endif
// *INDENT-ON*
	} while (0);
    } else {
// for cpm3 and cp/net we just try to select the drive
// if it returns 0xff we have a problem else all is o.k.
#ifdef OLD
	if (selectdrive(drive) == 0)
	    lres = 0;
	else
	    lres = 1;
#else
	lres = lbios(drive);
	printf("existlocal %c:%d\n", drive + 'A', lres);
	printf("reg  a: 0x%02x\n", areg);
	printf("reg bc: 0x%04x\n", bcreg);
	printf("reg de: 0x%04x\n", dereg);
	printf("reg hl: 0x%04x\n", hlreg);
#endif
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
	do {
// *INDENT-OFF*
#ifdef __SCCZ80
#asm
    ld      c,045h
    call    5
    ld      (_lres),hl
#endasm
#endif
#ifdef __SDCC
__asm
    ld      c,045h
    call    5
    ld      (_lres),hl
__endasm;
#endif
// *INDENT-ON*
	}
	while (0);
	// lres is a pointer to the cp/net drive table
	pp = (CONFIGTBL *) lres;
	lres = 0;
#ifdef DEBUG
	if (pp->disk[drive])
	    printf("%d is network drive\n", drive);
	else
	    printf("%d is local drive\n", drive);
#endif
	if (pp->disk[drive]) {
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
    do {
// *INDENT-OFF* 
#ifdef __SCCZ80
#asm
    ld      c,02dh
    ld      e,255
    jp      5
#endasm
#endif
#ifdef __SDCC
__asm
    ld      c,02dh
    ld      e,0ffh
    jp      5
__endasm;
#endif
// *INDENT-ON*
    } while (0);
    return;
}

int getversion()
{
// 22 = cp/m 2.2
// 31 = cp/m 3.1
// else cp/net

    TRACE(" getversion ");
    do {
//*INDENT-OFF*
#ifdef __SCCZ80
#asm
    ld      c,12
    call    5
    ld      (_version),hl
#endasm
#endif
#ifdef __SDCC
__asm
    ld      c,12
    call    5
    ld      (_version),hl
__endasm;
#endif
//*INDENT-ON*
    } while (0);
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
	    sprintf((char *) pname, "%c%d:%8s.%3s\0 ", disk, user, &name,
		    &ext);
	    logcount++;
	    return;
	}
	pname->next = (NAMEENTRY *) calloc(1, sizeof(NAMEENTRY));
	if (pname->next == 0) {
	    printf("Out of memory\n");
	    exit(1);
	}
	pname = pname->next;
	sprintf((char *) pname, "%c%d:%8s.%3s\0", disk, user, &name, &ext);
	logcount++;
	return;

    } else {
	printf("%c%d:%8s.%3s\n", disk, user, &name, &ext);
    }
}

// set the dma address
void setdma()
{
    TRACE(" setdma ");
    bdos(CPM_SDMA, (int) &diskbuf);
}

// set up the fcb for file search
void initfcb(unsigned char drive)
{
    TRACE(" initfcb ");
    parsefcb((struct fcb *) &Fcb, searchkey);
}

// get the first directory buffer
// searchkey is the prototype file name to look for.
int searchfirst()
{
    int ret;
    TRACE(" searchfirst ");
    ret = bdos(CPM_FFST, (int) &Fcb);
    TVAL("ret = %d\n", ret);
    return ret;
}

// get the next directory buffer
int searchnext()
{
    int ret;
    TRACE(" searchnext ");
    ret = bdos(CPM_FNXT, (int) 0);
    TVAL("ret=%d\n", ret);
    return ret;
}

// select the disk drive to search
int selectdrive(unsigned char drive)
{
    TRACE(" selectdrive ");
    ldrive = drive;
    do {
//*INDENT-OFF*
#ifdef __SCCZ80
#asm
    ld      c,14
    ld      a,(_ldrive)
    ld      e,a
    call    5
    ld      (_lres),hl
#endasm
#endif
#ifdef __SDCC
__asm
    ld      c,14
    ld      a,(_ldrive)
    ld      e,a
    call    5
    ld      (_lres),hl
__endasm;
#endif
//*INDENT-ON*
    } while (0);
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
	    printf(" process %c:\n ", drive_table[i] + 'A');
#endif
	    checkdrive(drive_table[i]);
	}
    }
}

extern int mygetopt(int, char **, void *);
extern void *optlist;
extern char *optarg;

void main(int argc, char *argv[])
{
    int opt;
    int i;
    memset(logfile, 0, 14);
    logp = NULL;
    version = getversion();
    if ((version & 0xff) == 0x31)
	seterrstat();
    if (argc >= 2) {
	while ((opt = mygetopt(argc, argv, &optlist)) != -1) {
	    switch (opt) {
	    case 4:		// -drive
		initdrivetab(optarg);
		break;
	    case 6:		// '.' (search all drives)
		initdrivetab(".");
		break;
	    case 1:		// -name 
		setsearchkey(optarg);
		break;
	    case 2:		// -output
		setlogname(optarg);
		break;
	    case 3:		// -user
		user = atoi(optarg);
		olduser = bdos(32, 0xff);
		bdos(32, user);
		break;
	    case 7:		// -alluser
		alluser++;
		olduser = bdos(32, 0xff);
		break;
	    case 5:		// -help
	    default:
		usage();
		exit(1);
		break;
	    }
	}
	if (alluser) {
	    for (i = 0; i < 15; i++) {
		user = i;
		bdos(32, i);	// cycle through all users
		Process();	// process the list.
	    }
	    bdos(32, olduser);
	} else
	    Process();
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
		fprintf(log, "%s\r", pname);
	    }
	    fclose(log);
	}
	if (user)
	    bdos(32, olduser);	// hope we don't crash
    }else {  
		usage();
    }
}
