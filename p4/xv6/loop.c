#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv) {
    int sleepT;
    if(argc != 2) { //take only one argument
	printf(1, "Invalid argument.\n");
	exit();
    }

    sleepT = atoi(argv[1]);
    //printf(1, "loop: sleeping for %d ticks\n", sleepT);
    sleep(sleepT);

    int i = 0, j = 0;
    while(i < 800000000) {
	j += i * j + 1;
	i++;
    }
    exit();
}
