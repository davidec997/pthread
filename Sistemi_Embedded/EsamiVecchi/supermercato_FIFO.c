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

struct cassa_t {
    int codac[C], oggetti [C];
    int head, tail;
    int noggetti;
    sem_t s;
}cassa;

struct supermercato_t {
    sem_t m;
    sem_t cliente[C];
    struct cassa_t casse [N];
}supermercato;

void myInit(struct supermercato_t *s)
{
    sem_init(&s->m,0,1);

    for (int i = 0; i < C; ++i) sem_init(&s->cliente[i],0,0);

    for (int i = 0; i < N; i++) {
        sem_init(&s->casse[i].s,0,0);
        s->casse[i].noggetti = 0;
        s->casse[i].head = 0;
        s->casse[i].tail = 0;
        for (int j = 0; j < C; j++) {
            s->casse->codac[j] = 0;
            s->casse->oggetti[j] = 0;
        }
    }
}

void pausetta(void){
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}



void cliente_pagamento (struct supermercato_t *s, int *pi, int noggetti){
    sem_wait(&s->m);

    int index = 0, min = s->casse[0].noggetti;
    for (int i = 1; i < N; i++) {
        if(s->casse[i].noggetti < min){
            index = i;
            min= s->casse[i].noggetti;
        }
    }

    printf("[CLIENTE %d]\tMI ACCODO ALLA CASSA %d CHE CHE DEVE PASSARE %d OGGETTI + I MIEI %d OGGETTI\n",*pi,index,s->casse[index].noggetti,noggetti);
    // mi accodo
    s->casse[index].noggetti += noggetti;
    int head = s->casse[index].head;
    s->casse[index].codac[head] = *pi;
    s->casse[index].oggetti[head] = noggetti;

    s->casse[index].head = (s->casse[index].tail + 1) % C;
    // ora spetto
    sem_post(&s->m);
    sem_post(&s->casse[index].s);
    sem_wait(&s->cliente[*pi]);   // wait

    // vengo svegliato quando e' il mio turno

    sem_wait(&s->m);
    // tolgo i miei ogg dalla cassa
    printf("[CLIENTE %d]\t\t\tSONO STATO SERVITO, TOLGO I MIEI %d OGGETTI\n",*pi,noggetti);

    s->casse[index].noggetti -= noggetti;
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

    sem_wait(&s->casse[*pi].s);
    sem_wait(&s->m);

    printf("\nSITUA\t");
    for (int i = 0; i < NUM_CASSIERI; i++) printf("%d\t",s->casse[i].noggetti);
    printf("\n\n");
    int tail = s->casse[*pi].tail;

    printf("[CASSIERE %d]\t\t Sto servendo il cliente %d che ha %d oggetti\n",*pi,s->casse[*pi].codac[tail],s->casse[*pi].oggetti[tail]);
    sem_post(&s->m);
    sleep(1);
}

void cassiere_fine_cliente (struct supermercato_t *s, int *pi) {
    sem_wait(&s->m);
    printf("[CASSIERE %d]\t\t\tHO SERVITO UN CLIENTE\n",*pi);
    int tail = s->casse[*pi].tail;
    int cliente_da_svegliare = s->casse[*pi].codac[tail];
    sem_post(&s->cliente[cliente_da_svegliare]);
    s->casse[*pi].tail = (s->casse[*pi].tail + 1) % C;// aggiorno tail

    sem_post(&s->m);
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



