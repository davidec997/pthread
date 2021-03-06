
//FUNZIONA
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define MAX_LICENZE 5
#define NTIMES 10
//#define K 7

typedef enum {false, true} Boolean;
pthread_cond_t vuoto,pieno;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int licenze;

void myInit(){
    pthread_cond_init(&vuoto,NULL);
    pthread_cond_init(&pieno,NULL);
    licenze = 15;
}

int richiedi_licenze(int index,int l,int iterazione){
    pthread_mutex_lock(&m);
    printf("\nSono il CLIENTE %d iterazione [%d] e devo richiedere %d  licenze\n",index,iterazione,l);

    while( licenze < l){
        pthread_cond_wait(&vuoto,&m);
    }
    licenze -= l;
    printf("\t SITUAZIONE ATTUALE\t %d LICENZE:\n",licenze);
    pthread_mutex_unlock(&m);
}

void rilascio_licenze(int l){
    pthread_mutex_lock(&m);
    licenze += l;
    pthread_mutex_unlock(&m);
    pthread_cond_broadcast(&vuoto);
}



void *eseguiUtente(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int r, licenze_richieste = 0;
    int iterazione=0;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (int i = 0; i < NTIMES ; i++) {
        r = rand() % MAX_LICENZE + 1;

        richiedi_licenze(*pi, r,iterazione);
        licenze_richieste += r;
        sleep(3);
        rilascio_licenze(r);

        printf("Thread %d iterazione [%d] HA OTTENUTO %d licenze e se ne va\n",*pi,iterazione,r);
        iterazione ++;
        sleep(2);

    }

    *ptr = licenze_richieste;
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

    myInit();
    srand(555);


    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiUtente, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d <- num di licenze richieste\n", i, ris);
    }

    pthread_mutex_destroy(&m);
    exit(0);
}



