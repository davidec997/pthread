/* OBIETTIVO: generare i 5 thread che rappresentano i filosofi: PROBLEMA DEI FILOSOFI RISOLTO CON UNA SOLUZIONE CHE USATA UN ARRAY DI SEMAFORI E UNA SOLUZIONE SIMMETRICA che chiaramente può portare al DEADLOCK!
 * La fase in cui il filosofo mangia e' stata simulata con una sleep mentre quella in cui pensa e' stata simulata con un descheduling.
 * Ogni thread torna al main il proprio numero d'ordine. */
#define _GNU_SOURCE             /* Per non avere warning utilizzando la pthread_yield - See feature_test_macros(7) */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define NTIMES 10

typedef enum {false, true} Boolean;

/* variabili globali */
sem_t S_BASTONCINO[5]; /* array di semafori ognuno con valore iniziale uguale a 1 */

void *eseguiFilosofo(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int i,j;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        printf("Problemi con l'allocazione dell'array ptr\n");
        exit(-1);
    }

    /* codice filosofo */
    printf("FILOSOFO con indice %d\n", *pi);

    if (*pi % 2 == 0)
    {
        /* codice filosofo pari */
        printf("FILOSOFO PARI con indice %d\n", *pi);

        for (i = 0; i < NTIMES; i++) /* while (true)  i filosofi dovrebbero essere cicli senza fine */
        {
            sem_wait(&(S_BASTONCINO[*pi]));
            sem_wait(&(S_BASTONCINO[(*pi+1)%5]));
            printf("FILOSOFO PARI con indice %d e identificatore %lu ha ottenuto di poter mangiare (i=%d)\n", *pi, pthread_self(),i);
            sleep(5); /* simuliamo l'azione di mangiare */
            sem_post(&(S_BASTONCINO[*pi]));
            sem_post(&(S_BASTONCINO[(*pi+1)%5]));
            printf("FILOSOFO PARI con indice %d e identificatore %lu ora pensa (i=%d)\n", *pi, pthread_self(), i);
            pthread_yield(); /* sleep(5); simuliamo l'azione di pensare */

        }
    }
    else
    {
        /* codice filosofo dispari */
        printf("FILOSOFO DISPARI con indice %d\n", *pi);

        for (j = 0; j < NTIMES; j++) /* while (true)  i filosofi dovrebbero essere cicli senza fine */
        {
            sem_wait(&(S_BASTONCINO[(*pi+1)%5]));
            sem_wait(&(S_BASTONCINO[*pi]));
            printf("FILOSOFO DISPARI con indice %d e identificatore %lu ha ottenuto di poter mangiare (j=%d)\n", *pi, pthread_self(),j);
            sleep(5); /* simuliamo l'azione di mangiare */
            sem_post(&(S_BASTONCINO[(*pi+1)%5]));
            sem_post(&(S_BASTONCINO[*pi]));
            printf("FILOSOFO DISPARI con indice %d e identificatore %lu ora pensa (j=%d)\n", *pi, pthread_self(), j);
            pthread_yield(); /* sleep(5); simuliamo l'azione di pensare */
        }
    }

    /* pthread torna al padre il valore intero dell'indice */
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main ()
{
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS = 5;

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

    /* prima di creare i thread, andiamo ad inizializzare i semafori, tutti al valore 1 */
    for (i=0; i < NUM_THREADS; i++)
    {
        if (sem_init(&(S_BASTONCINO[i]), 0, 1) != 0)
        {
            printf("Problemi con l'inizializzazione del semaforo %d-esimo\n", i);
            exit(5);
        }
    }

    /* creazione dei thread filosofi */
    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
       // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiFilosofo, (void *) (&taskids[i])) != 0)
            printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
        //free(p);
    }

    exit(0);
}

