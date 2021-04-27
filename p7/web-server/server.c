#include "helper.h"
#include "request.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>



// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

struct stat_struct {
  pthread_t TID;
  int static_requests;
  int dynamic_requests;
};

// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[])
{
  if (argc != 3) { //TEMP FOR THE POINTER ARG
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
}


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;

  getargs(&port, argc, argv);

  //
  // CS537 (Part B): Create & initialize the shared memory region...
  //
  int shm_fd = shm_open(argv[2], O_RDWR | O_CREAT, 0660); //CHANGE WHEN ARGS ARE IMPLEMENTED
  if(shm_fd < 0) {
    printf("ERROR: shm_open failed\n");
    exit(1);
  }
  ftruncate(shm_fd, getpagesize());


  //ACCESS SHARED PAGE MEMORY WITH THIS, CAN INDEX BY THREAD NUMBER
  struct stat_struct* shm_ptr =(struct stat_struct*)mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


  // //*****************TEST MEMORY ALLOC**********************//
  // for(int i = 0; i<32; i++) {
  //   shm_ptr[i].TID = 23452*i;
  //   shm_ptr[i].static_requests = i;
  //   shm_ptr[i].dynamic_requests = i%4;
  // }
  // while(1);

  // 
  // CS537 (Part A): Create some threads...
  //

  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    // 
    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. Also let the worker thread close the connection.
    // 
    requestHandle(connfd);
    Close(connfd);
  }
}
