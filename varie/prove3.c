#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define  N 10
sem_t Op1;
sem_t Op2;
sem_t m;
int turno = 1;
int *buff;

void *OP1(void * arg){
    printf("ciao sono op1 \n");

    int *pi = (int *)arg;
    int *ptr;
    int r,index;
    int posti_occupati = 0;

    ptr = (int *) malloc( sizeof(int));

    while(1){
        if  (turno == 1){
            sem_post( &Op1); // post previa

            sem_wait(&Op1);
            sem_wait(&m);
            printf("FACCIO OP1 \n");
            buff[*pi] = *pi;
            printf("%lu BUFFER iesimo\t\t size di un posto %d\n",buff[*pi],sizeof ((int*)buff[0]));
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
    int *pi = (int *)arg;
    int *ptr;
    int r,index;

    ptr = (int *) malloc( sizeof(int));
    while(1){
        if  (turno == 2){
            sem_post( &Op2); // post previa

            sem_wait(&Op2);
            sem_wait(&m);
            printf("FACCIO OP2 \n");
            buff[1] = *pi;
            printf("%lu BUFFER 0\n",buff[1]);
            sleep(1);
            turno = 1;
            sem_post(&m);

        }


    }
    //pthread_exit((void *) ID);
}


int main(int argc, char **argv){
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS;
    char error[250];

    //inizializzazioni vanno prima

    sem_init(&Op1, 0, 0);
    sem_init(&Op2,0,0);
    sem_init(&m,0,1);

    printf("%d\t%d\n\n",sizeof(int*),sizeof (int));
    buff = malloc(10* sizeof (int*));
    for (int j = 0; j < 10; j++) {
        buff[j] =  j;
        printf("%d\t",buff[j]);
    }



    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS = N;
    if (NUM_THREADS <= 0)
    {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
    }

    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    buff =   malloc(10* sizeof (int*));  //prova a cambiare questo

    if (taskids == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    srand(555);



    for (i=1; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, OP1, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=1; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d <- tot posti occupati\n", i, ris);
    }

    pthread_mutex_destroy(&m);
    exit(0);














    sleep(1);

    /*pthread_create(&thread, NULL, OP1, NULL);
    pthread_create(&thread, NULL, OP2, NULL);
    pthread_create(&thread, NULL, OP1, NULL);
    pthread_create(&thread, NULL, OP2, NULL);


    pthread_join(&thread,NULL);
    pthread_join(&thread,NULL);
    pthread_join(&thread,NULL);
    pthread_join(&thread,NULL);*/




}



