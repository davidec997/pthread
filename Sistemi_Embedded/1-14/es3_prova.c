//
// Created by davide on 02/01/22.
//   H           C           C
//  [ ] [] [1] [7] [6]  [8] [0] [9]  coda circolare
//  []  [] [0] [] []    [] [ 0] [1]  mi segno quelli con prio
//OK
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 10 // dimensione coda

typedef enum {false, true} Boolean;
//char *receivers [3] = {"RECEIVER A", "RECEIVER B", "RECEIVER C"};
typedef int T;

int valGlobale;
struct mailbox_t {
    int* coda_circolare;
    int head , tail;
    int n_msg;
    pthread_cond_t vuota,piena,letto_da_tutti,stop;
    pthread_mutex_t mtx;
    int *priorita ;       // 0 se non ha priorita e 1 se ha prio
    int *next
}mailbox;


void init_mailbox(struct mailbox_t *mb){

    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);

    pthread_mutex_init(&mb->mtx, NULL);
    mb->head = mb->tail = 0;
    mb->coda_circolare = malloc(N * sizeof (int*));
    mb->priorita = malloc(N * sizeof (int));

    for (int j = 0; j < N; ++j) {
        mb->coda_circolare[j] = (int*)-1;
        mb->priorita[j] = -1;

    }

    mb->n_msg = 0;
}

void send(struct mailbox_t *mb, int prio){
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
    if (prio)
        mb->priorita[mb->head] = 1;
    else
        mb->priorita[mb->head] = 0;

    printf("\n\n[SERVER]\t\tHO APPENA GENERATO IL MESSAGGIO '%d'E MESSO IN MAILBOX[%d]\n",msg,mb->head);
    mb->head = (mb->head + 1) % N;
    mb->n_msg ++;

    // introduci int blccati_su_vuota
    pthread_cond_broadcast(&mb->piena);

    pthread_mutex_unlock(&mb->mtx);
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
        send(&mailbox,rand()%2);
        //sleep(1);
    }
    *ptr = 0;
    pthread_exit((void *) ptr);
}



void leggi (struct mailbox_t *mb, int *pi){
    T msg;
    Boolean letto;
    //leggo

    // devo prendere prima quelli con priorita
    int index = -1;
    int c = mb->tail;

    while (1) {
        if (mb->priorita[c]) {
            index = c;
            break;
        }
        c = (c + 1) % N;
    }

    for (int i = mb->tail; i != mb->head ; i = (i + 1) % N) {
        if (mb->priorita[c]) {
            index = c;
            break;
        }
    }

    if (index == -1)
        index = mb->tail;
    // ho il primo con prio

    msg = mb->coda_circolare[index];
    printf("[RECEIVER %d]\t\t HO LETTO IL MESSAGGIO\t%d DALLA POSIZIONE\t[%d]\n",*pi,msg,mb->tail);


    //printf("[RECEIVER %d] MI SBLOCCO \n",*pi);
    pthread_cond_signal(&mb->vuota);

    pthread_cond_wait(&mb->stop,&mb->mtx);

    printf("[RECEIVER %d]\t\tRiparto \n",*pi);
}

void ricevi (struct mailbox_t *mb, int *pi){
    //controllo se c' e' qualcosa da leggere
    int * indice = (int*) pi;
    pthread_mutex_lock(&mb->mtx);

    while (mb->n_msg <= 0){
        printf("[RECEIVER %d] MAILBOX VUOTA\n",*pi);
        pthread_cond_wait(&mb->piena,&mb->mtx);
    }

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



