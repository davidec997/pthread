//
// Created by dada on 11/01/22.

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTIMES 3
#define N 5
#define C 25

typedef enum {false,true} Boolean;
int NUM_CASSIERI , NUM_CLIENTI ;

struct supermercato_t {
    sem_t m;
    sem_t cassiere[N], cliente[N];
    int oggetti_tot[N];

}supermercato;

void myInit(struct supermercato_t *s)
{
    sem_init(&s->m,0,1);
    for (int i = 0; i < NUM_CASSIERI; i++) {
        sem_init(&s->cassiere[i],0,0);
        sem_init(&s->cliente[i],0,0);

        s->oggetti_tot[i] = 0;
   }
}

void pausetta(void){
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}


int trovaCassiere (struct supermercato_t *s ){
    int index = 0;
    int min = 999;
    for (int i = 1; i < NUM_CASSIERI; i++) {
        //printf("\t\t\t\t%d OGGETTI\n",s->oggetti_tot[i]);
        if (s->oggetti_tot[i] < s->oggetti_tot[index]){
            index = i;
            //min = s->oggetti_tot[i];
        }
    }

    if (index < 0 || index > 10) printf("\tERRORR\n");
    printf("\tECCO IL TUO CAZZO DI INDICE DEL CAZZO %d\n",index);
    return  index;
}

void cliente_pagamento (struct supermercato_t *s, int *pi, int noggetti){
    sem_wait(&s->m);

    int index = 0, min = s->oggetti_tot[0];
    for (int i = 0; i < N; i++) {
        if(s->oggetti_tot[i] < min){
            index = i;
            min= s->oggetti_tot[i];
        }
    }

    printf("[CLIENTE %d]\tMI ACCODO ALLA CASSA %d CHE CHE DEVE PASSARE %d OGGETTI + I MIEI %d OGGETTI\n",*pi,index,s->oggetti_tot[index],noggetti);
    // mi accodo
    s->oggetti_tot[index] += noggetti;
    // ora spetto
    sem_post(&s->m);
    sem_post(&s->cassiere[index]);
    sem_wait(&s->cliente[index]);   // wait sulla coda clienti formatasi alla cassa index

    // vengo svegliato quando e' il mio turno

    sem_wait(&s->m);
    // tolgo i miei ogg dalla cassa
    printf("[CLIENTE %d]\t\t\tSONO STATO SERVITO, TOLGO I MIEI %d OGGETTI\n",*pi,noggetti);

    s->oggetti_tot[index]-= noggetti;
    sem_post(&s->m);

}

void *cliente(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =0;t<NTIMES;t++){
        sleep(rand()%20 +1);
        //printf("[CLIENTE %d]\tSono arrivato al supermercato e sto facendo la spesa\n",*pi);
        int oggetti = (rand()%30) + 1;
        pausetta();
        cliente_pagamento (&supermercato,pi,oggetti);
        //printf("[CLIENTE %d]\tHo pagato e vado a casa\n",*pi);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void cassiere_servo_cliente (struct supermercato_t *s , int *pi){

    sem_wait(&s->cassiere[*pi]);
    sem_wait(&s->m);

    printf("\nSITUA\t");
    for (int i = 0; i < NUM_CASSIERI; i++) printf("%d\t",s->oggetti_tot[i]);
    printf("\n\n");

    printf("[CASSIERE %d]\t\t Sto servendo un cliente\n",*pi);
    sem_post(&s->m);
    sleep(1);
}

void cassiere_fine_cliente (struct supermercato_t *s, int *pi) {
    sem_wait(&s->m);
    printf("[CASSIERE %d]\t\t\tHO SERVITO UN CLIENTE\n",*pi);
    sem_post(&s->m);
    sem_post(&s->cliente[*pi]);
}

void *cassiere(void *id) {
    int *pi = (int* ) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    // printf("indice %d\n",*pi);
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //  printf("\t\t\t\t\t\t\tCIAO SONO IL CASSIERE %d\n",*pi);

    for(;;){
        sleep(1);
        cassiere_servo_cliente(&supermercato,pi); // pi e' numerocassa
        //servo
        pausetta();
        cassiere_fine_cliente (&supermercato, pi);
    }
    *ptr = *pi;
    pthread_exit((void *) ptr);
}


int main (int argc, char **argv) {
    pthread_t *thread_cassieri;
    pthread_t *thread_clienti;

    int *taskidsCL, *taskidsCA;
    int i;
    int *p;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 3 ) /* Devono essere passati 2 parametri */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_CASSIERI = atoi(argv[2]);
    NUM_CLIENTI = atoi(argv[1]);
    if (NUM_CASSIERI <= 0 || NUM_CLIENTI <= 0)
    {
        sprintf(error, "Errore: Il primo o secondo parametro non e' un numero strettamente maggiore di 0\n");
        perror(error);
        exit(2);
    }

    myInit(&supermercato);

    sleep(2);
    thread_cassieri=(pthread_t *) malloc((NUM_CASSIERI) * sizeof(pthread_t));
    thread_clienti=(pthread_t *) malloc((NUM_CLIENTI) * sizeof(pthread_t));

    if (thread_cassieri == NULL || thread_clienti == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskidsCA = (int *) malloc(NUM_CASSIERI * sizeof(int));
    taskidsCL = (int *) malloc(NUM_CLIENTI * sizeof(int));

    if (taskidsCL == NULL || taskidsCA == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskidsCL o taskidsCA\n");
        exit(4);
    }

    // CREAZIONE PRIMA dei cassieri....
    for (i=0; i < NUM_CASSIERI; i++)
    {
        taskidsCA[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsCL[i]);
        if (pthread_create(&thread_cassieri[i], NULL, cassiere, (void *) (&taskidsCA[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsCL[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread AUTO %i-esimo con id=%lu\n", i, thread_cassieri[i]);
    }
    for (i= 0; i < NUM_CLIENTI ; i++)
    {
        taskidsCL[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsCL[i]);
        if (pthread_create(&thread_clienti[i], NULL, cliente, (void *) (&taskidsCL[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsCL[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread CAMPER %i-esimo con id=%lu\n", i, thread_clienti[i]);
    }

    sleep(2);


    for (i=0; i < NUM_CLIENTI; i++)
    {
        int ris;
        pthread_join(thread_clienti[i], (void**) & p);
        ris= *p;
        printf("Cliente %d-esimo restituisce %d \n", i, ris);
    }

    exit(0);
}


