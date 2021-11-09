#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

void main (){
    char s [5];
    int id = 3;
    sprintf(s,"%d\0",id);
    printf("questa e %s",s);
}