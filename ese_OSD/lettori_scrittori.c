//FUNZIONA DA VEDERE SOLO L-INDICE DEI THREAD
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 5
#define NTIMES 6
typedef enum {false,true} Boolean;
int lettori_attivi, lettori_bloccati;
int scrittori_attivi, scrittori_bloccati;
sem_t s_lettori, s_scrittori,m;
int risorsa;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&s_lettori,0,0);
    sem_init(&s_lettori,0,0);
    risorsa=0;
    lettori_attivi = lettori_bloccati = scrittori_attivi = scrittori_bloccati = 0;
}

void inizioLettura(){
    sem_wait(&m);
    if(!scrittori_attivi && !scrittori_bloccati){
        lettori_attivi ++;
        sem_post(&s_lettori); // post previa
    }else{
        lettori_bloccati ++;
    }
    sem_post(&m);
    sem_wait(&s_lettori);
}

void fineLettura(){
    sem_wait(&m);
    lettori_attivi --;
    if(!lettori_attivi && scrittori_bloccati){
        sem_post(&s_scrittori);
        scrittori_bloccati --;
        scrittori_attivi ++;
    }
    sem_post(&m);
}

void inizioScrittura(){
    sem_wait(&m);
    if(!scrittori_attivi && !lettori_attivi){
        sem_post(&s_scrittori);
        scrittori_attivi++;
    } else{
        scrittori_bloccati ++;
    }
    sem_post(&m);
    sem_wait(&s_scrittori);
}

void fineScrittura(){
    sem_wait(&m);
    scrittori_attivi --;
    if(lettori_bloccati){
        while(lettori_bloccati) {
            lettori_bloccati--;
            lettori_attivi++;
            sem_post(&s_lettori);
        }
    } else if (scrittori_bloccati){
        scrittori_bloccati--;
        scrittori_attivi ++;
        sem_post(&s_scrittori);
    }

    sem_post(&m);
}

void *eseguiScrittura(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =1;t<NTIMES;t++){
        inizioScrittura();
        risorsa += 1;
        printf("Thread %d sta SCRIVENDO la risorsa --> %d\n",*pi,risorsa);
        sleep(4);
        fineScrittura();
    }

    *ptr = pi;
    pthread_exit((void *) ptr);
}

void *eseguiLettura(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =1;t<NTIMES;t++){
        inizioLettura();
        printf("Thread %d sta LEGGENDO la risorsa --> %d\n",*pi,risorsa);
        sleep(4);
        fineLettura();
    }
    *ptr = NULL;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv)
{
    pthread_t *threadL;
    pthread_t *threadS;

    int *taskids;
    int i;
    int *p;
    int NUM_THREADS;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 2 ) /* Deve essere passato esattamente un parametro */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0)
    {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
    }

    myInit();

    threadL=(pthread_t *) malloc((NUM_THREADS/2) * sizeof(pthread_t));
    threadS=(pthread_t *) malloc((NUM_THREADS/2) * sizeof(pthread_t));

    if (threadL == NULL || threadS == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    for (i=0; i < NUM_THREADS/2; i++)
    {
        taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&threadL[i], NULL, eseguiLettura, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread LETTORE %i-esimo con id=%lu\n", i, threadL[i]);
    }
    for (i=NUM_THREADS/2; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&threadS[i], NULL, eseguiScrittura, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread SCRITTORE %i-esimo con id=%lu\n", i, threadS[i]);
    }

    sleep(5);
    for (i=0; i < NUM_THREADS/2; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(threadL[i], (void**) & p);
        ris= *p;
        printf("Pthread Lettore %d-esimo restituisce %d\n", i, ris);
    }
    for (i=NUM_THREADS/2; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(threadS[i], (void**) & p);
        ris= *p;
        printf("Pthread Scrittore %d-esimo restituisce %d\n", i, ris);
    }

    exit(0);
}


