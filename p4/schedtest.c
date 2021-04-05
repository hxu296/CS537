#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char **argv) {
    int id;
    int idB;
    int sliceA;
    int sleepA;
    int sliceB;
    int sleepB;
    int sleepParent;
    struct pstat info;
    if(argc != 6) { //receive 5 arguments
	printf(1, "invalid argument.\n");
	exit();
    }
    sliceA = atoi(argv[1]);
    sleepA = atoi(argv[2]);
    sliceB = atoi(argv[3]);
    sleepB = atoi(argv[4]);
    sleepParent = atoi(argv[5]);

    //start of children
    id = fork2(sliceA);
    if(id == 0) { //child A
	char *args[] = {"loop", argv[2], NULL};//call loop for A
	exec(args[0], args); 
        //if child reached here, exec failed
	printf(1, "childA exec failed.\n");
	exit();
    } else { //parent fork B
          idB = fork2(sliceB);
	  if(idB == 0) { //child B
	      char *argb[] = {"loop", argv[4], NULL}; //call loop for B
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
	  //find the compticks of processA and B
	  int compticksA = info.compticks[0];
	  int compticksB = info.compticks[1];
	  printf(1, "%d %d\n", compticksA, compticksB);
      }

    //wait 
    wait();
    wait();

    exit();
}




