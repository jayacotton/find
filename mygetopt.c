#include <stdlib.h>
#include <string.h>

// all the things that can be sent to find.
// A-P  Drive letters.
// .  All drives
// -NAME <name string>
// -OUTPUT < file name for log >
// -USER <number>
// -HELP

char *optlist[7][3] = {
    { "-N", 2, 1 },             // -name
    { "-O", 2, 2 },             // -output
    { "-U", 2, 3 },             // -user
    { "-D", 2, 4 },             // -drive
    { "-H", 1, 5 },             // -help
    { ".", 1, 6 },              // .
    { "-A", 1, 7 }              // -alluser
};

#define LENLIST 7
int getoptindex = 1;
char *optarg;

int mygetopt(int argc, char *argv[], char *list[LENLIST][3])
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
        }                       // so it must be a '-'
        if (strstr(argv[getoptindex], optlist[i][0])) {
            optarg = argv[getoptindex + 1];
            getoptindex += (int) optlist[i][1];
            return ((int) optlist[i][2]);
        }
    }
    return (-1);
}
