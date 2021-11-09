// produttori consumatori con stringa [5]. Esercitazione del 13 Novembre 2020
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define N 5
typedef struct {
    char dato [5];
    int iter;
} Messaggio;

Messaggio  buffer [N];
sem_t vuoto, pieno, m;
int head, tail;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&vuoto,0,N);
    sem_init(&pieno,0,0);
    head = tail = 0;

}

void *producer(void *arg)
{
    int *pi = (int *) arg;
    Messaggio p;
    //sleep(1);
    for (int i = 0; i < 100; i++) {
        sem_wait(&vuoto);
        sem_wait(&m);
        sprintf(p.dato,"%lu\0",*pi);
        p.iter = i;
        buffer[head] = p;
        printf("PRODOTTO e messo nel buffer il dato \t%s Iterazione\t%d\tdalla posizione [%d]\n",p.dato,p.iter,head);
        head = (head + 1) % N;
        sem_post(&pieno);
        sem_post(&m);
        pausetta();
    }
    pthread_exit(0);
}

void *consumer(void *arg)
{
    int *pi = (int *) arg;
    Messaggio p;

    for (int i = 0; i < 100; i++) {
        sem_wait(&pieno);
        sem_wait(&m);
        p = buffer[tail];
        printf("LETTO e PRELEVATO  dal buffer  il dato \t%s Iterazione\t%d\tdalla posizione [%d]\n",p.dato,p.iter,tail);
        tail = (tail + 1)% N;
        sem_post(&vuoto);
        sem_post(&m);
        pausetta();
    }
    pthread_exit(0);

}


int main(int argc, char *argv[]) {
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS;

    /* Controllo sul numero di parametri */
    if (argc != 2 ) /* Deve essere passato esattamente un parametro */
    {
        printf("Errore nel numero dei parametri %d\n", argc-1);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0)
    {
        printf("Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        exit(2);
    }

    myInit();

    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
    {
        printf("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        printf("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    for (i=0; i < NUM_THREADS/2; i++)
    {
        taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, producer, (void *) (&taskids[i])) != 0)
            printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
        printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=NUM_THREADS/2; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, consumer, (void *) (&taskids[i])) != 0)
            printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
        printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    exit(0);

}

