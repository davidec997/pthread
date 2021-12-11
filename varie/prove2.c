#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

sem_t Op1;
sem_t Op2;
sem_t m;
int turno = 1;

void *OP1(void * arg){
    printf("ciao sono op1 \n");


    while(1){
        if  (turno == 1){
            sem_post( &Op1); // post previa

            sem_wait(&Op1);
            sem_wait(&m);
            printf("FACCIO OP1 \n");
            sleep(1);
            turno = 2;
            sem_post(&m);
        }

            //sem_post(&Op2);
           // printf("op1 2\n");

    }

    //pthread_exit((void *) ID);
}


void *OP2(void * arg){
    printf("ciao sono op2 \n");

    while(1){
        if  (turno == 2){
            sem_post( &Op2); // post previa

            sem_wait(&Op2);
            sem_wait(&m);
            printf("FACCIO OP2 \n");
            sleep(1);
            turno = 1;
            sem_post(&m);

        }


    }
    //pthread_exit((void *) ID);
}


int main(int argc, char **argv){
    int i=0;
    pthread_t thread;

    //inizializzazioni vanno prima

    sem_init(&Op1, 0, 0);
    sem_init(&Op2,0,0);
    sem_init(&m,0,1);
    sleep(1);

    pthread_create(&thread, NULL, OP1, NULL);
    pthread_create(&thread, NULL, OP2, NULL);

    pthread_join(&thread,NULL);
    pthread_join(&thread,NULL);



}



