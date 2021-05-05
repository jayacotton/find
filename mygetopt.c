#include <stdlib.h>
#include <string.h>

// all the things that can be sent to find.
// A-P  Drive letters.
// .  All drives
// -NAME <name string>
// -OUTPUT < file name for log >
// -USER <number>
// -HELP

char *optlist[6][2] = {
    { "-N", 1 },
    { "-O", 2 },
    { "-U", 3 },
    { "-D", 4 },
    { "-H", 5 },
    { ".", 6 }
};

#define LENLIST 6
int getoptindex = 1;
char *optarg;

int mygetopt(int argc, char *argv[], char *list[LENLIST][2])
{
// given argv[x] find that in list[n][0]
// when found return a token 
//  the token will have to be the index of list starting at n+1
    int i;
    if (getoptindex > argc)
	return (-1);
    for (i = 0; i < LENLIST; i++) {
// if the arg does not have a '-' then is it '.'
	if (strchr(argv[getoptindex], '.')) {
	    getoptindex++;
	    return (6);
	}			// so it must be a '-'
	if (strstr(argv[getoptindex], optlist[i][0])) {
	    optarg = argv[getoptindex + 1];
	    getoptindex += 2;
	    return (optlist[i][1]);
	}
    }
    return (-1);
}
