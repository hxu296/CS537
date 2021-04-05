#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "param.h"

void printinfo(struct pstat info){
    printf(1, "inuse: ");
    for(int i = 0; i < NPROC; i++){
        printf(1, "%d ", info.inuse[i]);
    }
    printf(1, "\n");
    printf(1, "pid: ");
    for(int i = 0; i < NPROC && info.inuse[i]; i++){
        printf(1, "%d ", info.pid[i]);
    }
    printf(1, "\n");
    printf(1, "schedticks: ");
    for(int i = 0; i < NPROC && info.inuse[i]; i++){
        printf(1, "%d ", info.schedticks[i]);
    }
    printf(1, "\n");
    printf(1, "sleepticks: ");
    for(int i = 0; i < NPROC && info.inuse[i]; i++){
        printf(1, "%d ", info.sleepticks[i]);
    }
    printf(1, "\n");
    printf(1, "switches: ");
    for(int i = 0; i < NPROC && info.inuse[i]; i++){
        printf(1, "%d ", info.switches[i]);
    }
    printf(1, "\n");
    printf(1, "compticks: ");
    for(int i = 0; i < NPROC && info.inuse[i]; i++){
        printf(1, "%d ", info.compticks[i]);
    }
    printf(1, "\n");
}

int
main(int argc, char **argv) {
    int idA;
    int idB;
    int sliceA;
    char* sleepA;
    int sliceB;
    char* sleepB;
    int sleepParent;
    struct pstat info;
    if(argc != 6) { //receive 5 arguments
	printf(1, "invalid argument.\n");
	exit();
    }
    sliceA = atoi(argv[1]);
    //printf(1, "aliceA: %d\n", sliceA);
    sleepA = argv[2];
    //printf(1, "sleepA: %s\n", sleepA);
    sliceB = atoi(argv[3]);
    //printf(1, "aliceB: %d\n", sliceB);
    sleepB = argv[4];
    //printf(1, "sleepB: %s\n", sleepB);
    sleepParent = atoi(argv[5]);

    //start of children
    idA = fork2(sliceA);
    if(idA == 0) { //child A
	char *arga[] = {"loop", sleepA, 0}; //call loop for A
	exec(arga[0], arga);
        //if child reached here, exec failed
	printf(1, "childA exec failed.\n");
	exit();
    } else { //parent fork B
          idB = fork2(sliceB);
	  if(idB == 0) { //child B
	      char *argb[] = {"loop", sleepB, 0}; //call loop for B
	      exec(argb[0], argb);
	      //if child reached here exec failed
	      printf(1, "childB exec failed.\n");
	      exit();
	  }
      }
    //parent sleep
    sleep(sleepParent);
    if(getpinfo(&info) == -1) { //extract info fail
	printf(1, "fail to extract the info about processes.\n");
	exit();
    } else {
        int compticksA = -1, compticksB = -1;
        for(int i = 0; i < NPROC; i++){
            if(info.pid[i] == idA)
                compticksA = info.compticks[i];
            else if(info.pid[i] == idB)
                compticksB = info.compticks[i];
        }
        //printinfo(info);
        //printf(1, "idA: %d, idB: %d\n", idA, idB);
	   printf(1, "%d %d\n", compticksA, compticksB);

      }

    //wait 
    wait();
    wait();

    exit();
}




