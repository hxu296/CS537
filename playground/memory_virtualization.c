#include <stdio.h>
#include <stdlib.h>

#define LOOPS 10 

void main(){
    for(int i = 0; i < LOOPS; i++){
        printf("malloc #%d: %p\n", i + 1, malloc(sizeof(int)));
    }
}

