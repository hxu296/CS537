#include "helper.h"
#include "request.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>


struct stat_struct {
  pthread_t TID;
  int static_requests;
  int dynamic_requests;
};


int main(int argc, char**argv) {
    if(argc != 4) {
        printf("ERROR: Wrong number of args\n");
        exit(1);
    }

    int shm_fd = shm_open(argv[1], O_RDWR, 0660);
    if(shm_fd < 0) {
        printf("ERROR: shm_open failed\n");
        exit(1);
    }


    //CAN BE INDEXED BY THREAD
    struct stat_struct* shm_ptr =(struct stat_struct*)mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    int sleepytime = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    if(sleepytime<0) {
        printf("ERROR: Pick a better number for sleeptime_ms\n");
        exit(1);
    }
    if(num_threads<0 || num_threads>32) {
        printf("ERROR: Pick a better number for numthreads\n");
        exit(1);
    }

    struct timespec spec;
    long nanomult = 1000000;
    spec.tv_sec = 0;
    spec.tv_nsec = (long)sleepytime * nanomult;

    int j = 0;
    while(1) {
        nanosleep(&spec, NULL);
        printf("\n%d\n", j);
        for(int i = 0; i<num_threads; i++) {
            
            printf("%ld : %d %d %d\n", shm_ptr[i].TID, shm_ptr[i].static_requests+shm_ptr[i].dynamic_requests,shm_ptr[i].static_requests, shm_ptr[i].dynamic_requests);
        }
        j++;
    }

    
}

