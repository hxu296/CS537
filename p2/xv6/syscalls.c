#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char **argv)
{
    int N, g, pid;

    if (argc != 3){
        printf(2, "usage: syscalls N g\n");
        exit();
    }

    N = atoi(argv[1]);
    g = atoi(argv[2]);
    pid = getpid();

    for(; N-g > 0; N--)
        kill(-1);

    for(; g > 1; g--)
        getpid();

    printf(1, "%d %d\n", getnumsyscalls(pid), getnumsyscallsgood(pid));
    exit();
}

