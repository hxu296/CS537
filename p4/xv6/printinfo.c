#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(void) {
    struct pstat pst;
    if(getpinfo(&pst) == 0)
        printf(1, "getpinfo succeed\n");
    else {
        printf(1, "getpinfo failed\n");
        exit();
    }
    printf(1, "inuse: ");
    for(int i = 0; i < NPROC; i++){
        printf(1, "%d ", pst.inuse[i]);
    }
    printf(1, "\n");
    printf(1, "pid: ");
    for(int i = 0; i < NPROC && pst.inuse[i]; i++){
        printf(1, "%d ", pst.pid[i]);
    }
    printf(1, "\n");
    printf(1, "schedticks: ");
    for(int i = 0; i < NPROC && pst.inuse[i]; i++){
        printf(1, "%d ", pst.schedticks[i]);
    }
    printf(1, "\n");
    printf(1, "sleepticks: ");
    for(int i = 0; i < NPROC && pst.inuse[i]; i++){
        printf(1, "%d ", pst.sleepticks[i]);
    }
    printf(1, "\n");
    printf(1, "switches: ");
    for(int i = 0; i < NPROC && pst.inuse[i]; i++){
        printf(1, "%d ", pst.switches[i]);
    }
    printf(1, "\n");
    printf(1, "compticks: ");
    for(int i = 0; i < NPROC && pst.inuse[i]; i++){
        printf(1, "%d ", pst.compticks[i]);
    }
    printf(1, "\n");
    exit();
}
