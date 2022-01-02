//
// Created by davide on 02/01/22.
//
//ok
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NTIMES 30

#define N 10 // dimensione coda; max macchine ammesse nella coda

typedef enum {false, true} Boolean;
char *senders [3] = {"SENDER P1", "SEENDER P2", "SENDER P3"};
typedef int T;

int valGlobale;

struct semaforoprivato_t {
    pthread_cond_t s;
    int c;
};

struct mailbox_t {
    int* coda_circolare;
    int head , tail;
    int n_msg;
    pthread_cond_t vuota,piena,stop;
    pthread_mutex_t mtx;
    struct semaforoprivato_t priv[3];
}mailbox;


struct busta_t{
    T messaggio;
    int priorita;
}busta;

Boolean check_lettura (struct mailbox_t *mb);

void semaforoprivato_init(struct semaforoprivato_t *s)
{
    pthread_cond_init(&s->s,NULL);
    s->c = 0;
}

void init_mailbox(struct mailbox_t *mb){

    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);
    //pthread_cond_init(&mb->nuovo_msg, NULL);
    pthread_cond_init(&mb->stop, NULL);

    pthread_mutex_init(&mb->mtx, NULL);
    mb->head = mb->tail = 0;
    mb->coda_circolare = malloc(N * sizeof (int*));
    for (int j = 0; j < N; ++j) mb->coda_circolare[j] = (int*)-1;
    mb->n_msg = 0;

    for (int f = 0; f < 3; f++) semaforoprivato_init(&mb->priv[f]);

}


void genera_msg(struct mailbox_t *mb, int prio){
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
    printf("\n\n[SERVER]\t\tHO APPENA GENERATO IL MESSAGGIO '%d'E MESSO IN MAILBOX[%d]\n",msg,mb->head);
    mb->head = (mb->head + 1) % N;
    mb->n_msg ++;

    pthread_cond_broadcast(&mb->piena);

    pthread_mutex_unlock(&mb->mtx);



    sleep(4);
    pthread_mutex_lock(&mb->mtx);


    mb->tail = (mb->tail + 1) % N;
    mb->n_msg --;
    pthread_mutex_unlock(&mb->mtx);
    pthread_cond_broadcast(&mb->stop);

}

void *generatore (void * id, int prio){
    int *pi = (int *)id;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    while(1){
        genera_msg(&mailbox,prio);
        //sleep(1);
    }
    *ptr = 0;
    pthread_exit((void *) ptr);
}




void leggi (struct mailbox_t *mb, int *pi){
    T msg;
    Boolean letto;
    //leggo

    msg = mb->coda_circolare[mb->tail];
    printf("[RECEIVER %d]\t\t HO LETTO IL MESSAGGIO\t%d DALLA POSIZIONE\t[%d]\n",*pi,msg,mb->tail);


    //printf("[RECEIVER %d] MI SBLOCCO \n",*pi);
    pthread_cond_signal(&mb->vuota);
    pthread_cond_wait(&mb->stop,&mb->mtx);

    printf("[RECEIVER %d]\t\tRiparto \n",*pi);
}

void ricevi (struct mailbox_t *mb, int *pi){
    //controllo se c' e' qualcosa da leggere
    int * indice = (int*) pi;

    //printf("ECCO IL MIO INDICE \t\t\t\t\t%d\n",*indice );

    pthread_mutex_lock(&mb->mtx);
    while (mb->n_msg <= 0){
        printf("[RECEIVER %d] MAILBOX VUOTA\n",*pi);
        pthread_cond_wait(&mb->piena,&mb->mtx);
    }
    //leggo
    leggi(&mailbox,indice);

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
        ricevi(&mailbox,pi);
        sleep(1);
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
    for (i=0; i < NUM_THREADS; i++) {
        taskids[i] = i;
        if (i == NUM_THREADS - 1) {

            pthread_create(&thread[i], NULL, generatore, (void *) (&taskids[i]));
        } else {
            if (pthread_create(&thread[i], NULL, receiver, (void *) (&taskids[i])) != 0) {
                sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",
                        taskids[i]);
                perror(error);
                exit(5);
            }
        }
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





