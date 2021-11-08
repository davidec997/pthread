
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int x = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;


void *body(void *id)
{
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    pthread_mutex_lock(&mtx);

    for(int t =0; t<10; t++){
        x += 1;
        printf("Thread%d con identificatore %lu  --> valore di x: %d\n", *pi, pthread_self(),x);
    }

    pthread_mutex_unlock(&mtx);


    /* pthread vuole tornare al padre un valore intero, ad es 1000+id */
    *ptr = 1000+*pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv)
{
    pthread_t *thread;
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

    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
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

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, body, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    pthread_mutex_destroy(&mtx);
    exit(0);
}

