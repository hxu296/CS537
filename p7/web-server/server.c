#include "helper.h"
#include "request.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define EMPTY 0
#define FULL 1
#define IN_PROGRESS 2
#define NUM_BUFFERS 100

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

typedef struct stat_struct {
  pthread_t TID;
  int static_requests;
  int dynamic_requests;
}slot_t;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t full;
    pthread_cond_t empty;
    int num_full;
    int num_empty;
    pthread_t consumer_buffer[NUM_BUFFERS];
    pthread_t producer_buffer[NUM_BUFFERS];
    int requests_buffer[NUM_BUFFERS];
    int buffer_status[NUM_BUFFERS];
    slot_t* shm_slot_ptr;
}thread_arg;

// server related.
int listenfd;
int num_buffers;
int num_threads;
int port;
char* shm_name;

//memory pointer
void* shm_ptr;
void record_stat(void* arg_ptr, int request_type){
    thread_arg *a = (thread_arg*)arg_ptr;
    pthread_t TID = pthread_self();
    // find pthread shm slot.
    int shm_index = 0;
    for(; shm_index < num_threads; shm_index++)
        if(TID == a->consumer_buffer[shm_index]) break;
    // record relevant statistics.
    a->shm_slot_ptr[shm_index].TID = TID;
    if(request_type == DYNAMIC)
        a->shm_slot_ptr[shm_index].dynamic_requests++;
    else if(request_type == STATIC)
        a->shm_slot_ptr[shm_index].static_requests++;
}

void produce_request(void* arg_ptr, int empty_index){
    thread_arg *a = (thread_arg*)arg_ptr;
    int connfd, clientlen;
    struct sockaddr_in clientaddr;
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
    a->requests_buffer[empty_index] = connfd;
    a->buffer_status[empty_index] = FULL;
}

void consume_request(void* arg_ptr, int full_index){
    thread_arg *a = (thread_arg*)arg_ptr;
    int connfd = a->requests_buffer[full_index];
    int request_type = requestHandle(connfd);
    Close(connfd);
    record_stat(arg_ptr, request_type);
    a->buffer_status[full_index] = EMPTY;
}

int get_empty(void* arg_ptr){
    thread_arg *a = (thread_arg*)arg_ptr;
    int empty_index;
    for(int i = 0; i < num_buffers; i++){
        if(a->buffer_status[i] == EMPTY) {
            empty_index = i;
            break;
        }
    }
    a->buffer_status[empty_index] = IN_PROGRESS;
    a->num_empty--;
    return empty_index;
}

int get_full(void* arg_ptr){
    thread_arg *a = (thread_arg*)arg_ptr;
    int full_index;
    for(int i = 0; i < num_buffers; i++){
        if(a->buffer_status[i] == FULL){
            full_index = i;
            break;
        }
    }
    a->buffer_status[full_index] = IN_PROGRESS;
    a->num_full--;
    return full_index;
}

void *produce(void *arg_ptr){
    thread_arg *a = (thread_arg*)arg_ptr;
    int empty_index;
    while(1){
        pthread_mutex_lock(&a->mutex);
        while(a->num_empty == 0)
            pthread_cond_wait(&a->empty, &a->mutex);
        empty_index = get_empty(arg_ptr);
        pthread_mutex_unlock(&a->mutex);

        produce_request(arg_ptr, empty_index);

        pthread_mutex_lock(&a->mutex);
        a->num_full++;
        pthread_cond_signal(&a->full);
        pthread_mutex_unlock(&a->mutex);
    }
}

void *consume(void *arg_ptr){
    thread_arg *a = (thread_arg*)arg_ptr;
    int full_index;
    while(1){
        pthread_mutex_lock(&a->mutex);
        while(a->num_full == 0)
            pthread_cond_wait(&a->full, &a->mutex);
        full_index = get_full(arg_ptr);
        pthread_mutex_unlock(&a->mutex);

        consume_request(arg_ptr, full_index);

        pthread_mutex_lock(&a->mutex);
        a->num_empty++;
        pthread_cond_signal(&a->empty);
        pthread_mutex_unlock(&a->mutex);
    }
}

void error_exit(char *argv[]){
    fprintf(stderr, "Usage: %s <port> <threads> <buffers> <shm_name>\n", argv[0]);
    exit(1);
}


void sigIntHandler(int sig_num) {
  munmap((char *) shm_ptr, getpagesize());
  shm_unlink(shm_name);
  exit(0);
}


int main(int argc, char *argv[])
{
    // parse arguments.
    if (argc != 5) error_exit(argv);

    if((port = atoi(argv[1])) == 0 || port < 0) error_exit(argv);
    if((num_threads = atoi(argv[2])) == 0 || num_threads < 0) error_exit(argv);
    if((num_buffers = atoi(argv[3])) == 0 || num_buffers < 0) error_exit(argv);

    shm_name = argv[4];

  //printf("DEBUG: port: %d  num_threads: %d  num_buffers: %d  shm_name: %s\n", port, num_threads, num_buffers, shm_name);
    signal(SIGINT, sigIntHandler);
  //
  // CS537 (Part B): Create & initialize the shared memory region...
  //
  int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0660); //CHANGE WHEN ARGS ARE IMPLEMENTED
  if(shm_fd < 0) {
    printf("ERROR: shm_open failed\n");
    exit(1);
  }
  ftruncate(shm_fd, getpagesize());


  //ACCESS SHARED PAGE MEMORY WITH THIS, CAN INDEX BY THREAD NUMBER
  shm_ptr =mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  slot_t* shm_slot_ptr = (slot_t*) shm_ptr;

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

  // initialize thread_arg
  thread_arg arg;
  arg.num_full = 0;
  arg.num_empty = num_buffers;
  arg.shm_slot_ptr = shm_slot_ptr;
  memset(arg.buffer_status, EMPTY, NUM_BUFFERS * sizeof(int));
  listenfd = Open_listenfd(port);
  pthread_mutex_init(&arg.mutex, NULL);
  pthread_cond_init(&arg.empty, NULL);
  pthread_cond_init(&arg.full, NULL);
  // create producer and consumer threads.
  for(int i = 0; i < num_threads; i++)
      pthread_create(arg.consumer_buffer + i, NULL, consume, &arg);
  for(int i = 0; i < num_buffers; i++)
      pthread_create(arg.producer_buffer + i, NULL, produce, &arg);

  // wait for consumers and producers.
  for(int i = 0; i < num_threads; i++)
      pthread_join(arg.consumer_buffer[i], NULL);
  for(int i = 0; i < num_buffers; i++)
      pthread_join(arg.producer_buffer[i], NULL);

  Close(listenfd);
  return 0;
}
