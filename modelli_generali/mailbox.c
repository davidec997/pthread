//
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NTIMES 30

#define N 10 // dimensione coda

typedef enum {false, true} Boolean;
char *receivers [3] = {"RECEIVER A", "RECEIVER B", "RECEIVER C"};
typedef int T;

int valGlobale;
struct mailbox_t {
    int* coda_circolare;
    int head , tail;
    int n_msg;
    pthread_cond_t vuota,piena;
    pthread_mutex_t mtx;
}mailbox;


void init_mailbox(struct mailbox_t *mb){

    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);

    pthread_mutex_init(&mb->mtx, NULL);
    mb->head = mb->tail = 0;
    mb->coda_circolare = malloc(N * sizeof (int*));
    for (int j = 0; j < N; ++j) mb->coda_circolare[j] = (int*)-1;
    mb->n_msg = 0;

}


void genera_msg(struct mailbox_t *mb){
    //controllo se c'e' posto nella mailbox
    pthread_mutex_lock(&mb->mtx);
    while (mb->n_msg >= N){
        //aspetto perche' la mailbox e' piena
        printf("[SERVER] MAILBOX PIENA\n");
        pthread_cond_wait(&mb->vuota ,&mb->mtx);
    }
    //posso generare il msg
    T msg = valGlobale;
    valGlobale ++;
    mb->coda_circolare[mb->head] = msg;
    printf("[SERVER]\t\tHo appena generato il msg %d e messo in mb[%d]\n",msg,mb->head);
    mb->head = (mb->head + 1) % N;
    mb->n_msg ++;
    pthread_mutex_unlock(&mb->mtx);
    pthread_cond_broadcast(&mb->piena);
}

void *generatore (void * id){
    int *pi = (int *)id;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    while(1){
        sleep(1);
        genera_msg(&mailbox);
    }
    *ptr = 0;
    pthread_exit((void *) ptr);
}


void leggi (struct mailbox_t *mb, int *pi){
    T msg;
    Boolean letto;
    //leggo
    pthread_cond_wait(&mb->nuovo_msg,&mb->mtx);

    msg = mb->coda_circolare[mb->tail];
    printf("[RECEIVER %d]\t\t Ho letto il messaggio\t%d dalla posizione\t[%d]\n",*pi,msg,mb->tail);

    mb->tail = (mb->tail + 1) % N;
    mb->n_msg --;

}

void ricevi (struct mailbox_t *mb, int *pi){
    //controllo se c' e' qualcosa da leggere
    //printf("ECCO IL MIO INDICE \t\t\t\t\t%d\n",*pi );

    pthread_mutex_lock(&mb->mtx);
    while (mb->n_msg <= 0){
        printf("[RECEIVER %d] MAILBOX VUOTA\n",*pi);
        pthread_cond_wait(&mb->piena,&mb->mtx);
    }
    //leggo
    leggi(&mailbox,pi);

    pthread_mutex_unlock(&mb->mtx);
}

void *receiver (void *id) {
    int *pi = (int *)id;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    int n_attraversamenti=0;
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    while (1){
        // printf("ECCO IL MIO INDICE \t%d\n",*pi);
        sleep(3);

        ricevi(&mailbox,pi);
    }
    *ptr = 0;
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

    init_mailbox(&mailbox);

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


    srand(555);

    sleep(2);

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        if (i == NUM_THREADS -1){

            pthread_create(&thread[i], NULL, generatore, (void *) (&taskids[i]));
        } else {
            if (pthread_create(&thread[i], NULL, receiver, (void *) (&taskids[i])) != 0)
            {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(5);
            }
        }

        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    exit(0);
}




