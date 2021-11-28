#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef enum {false, true} Boolean;

void main (){

    Boolean var = false;
    for (int i = 0; i <10; i++) {
        printf("%d\n",var);
        var = !var;
    }

   /* char s [5];
    int id = 3;
    sprintf(s,"%d\0",id);
    printf("questa e %s",s);*/
/*
   int soldi = {0,0,0};
   soldi[2] = 5;
   soldi[0] = -9;
    printf(" %d, %d",soldi, soldi[2]);*/

}