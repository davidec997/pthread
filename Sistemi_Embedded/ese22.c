
// devo farlo senza ciclo.. e fare che una macchina che e' arrivata dopo ad un incrocio, non puo passare se quella che la precede non ha attraversato
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NTIMES 30

#define N 10 // dimensione coda; max macchine ammesse nella coda

typedef enum {false, true} Boolean;
char *receivers [3] = {"RECEIVER A", "RECEIVER B", "RECEIVER C"};
typedef int T;

int valGlobale;
struct mailbox_t {
    int stato;
    int* coda_circolare;
    int head , tail;
    int n_msg;
    pthread_cond_t vuota,piena,letto_da_tutti,nuovo_msg,stop;
    pthread_mutex_t mtx;
    int conferma_lettura[3];
 }mailbox;
/*

struct busta_t{
    T messaggio;
    int priorita;
}busta;
*/

void init_mailbox(struct mailbox_t *mb){

    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);
    pthread_cond_init(&mb->letto_da_tutti, NULL);
    pthread_cond_init(&mb->nuovo_msg, NULL);
    pthread_cond_init(&mb->stop, NULL);

    pthread_mutex_init(&mb->mtx, NULL);
    mb->stato = -1;
    mb->head = mb->tail = 0;
    mb->coda_circolare = malloc(N * sizeof (int*));
    for (int j = 0; j < N; ++j) mb->coda_circolare[j] = (int*)-1;
    mb->conferma_lettura[0] = mb->conferma_lettura[1] = mb->conferma_lettura[2] = 0;
    mb->n_msg = 0;

}


/*
void sveglia_bloccati(struct mailbox_t *s){
    // il semaforo verde si occupa di svegliare la  macchina se e' bloccata

    pthread_cond_broadcast(&s->stop);

}*/

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
    printf("[SERVER]\t\tHo appena generato un msg e messo in mb[%d]\n",mb->head);
    mb->head = (mb->head + 1) % N;
    mb->n_msg ++;

    pthread_cond_broadcast(&mb->piena);
    pthread_cond_broadcast(&mb->nuovo_msg);

    pthread_cond_broadcast(&mb->letto_da_tutti);


    //ora spetto che tutti e 3 i receivers abbiano letto il msg
    // while(!check_lettura(&mailbox))
    //pthread_cond_wait(&mb->letto_da_tutti,&mb->mtx);

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
        sleep(1);
        genera_msg(&mailbox);
    }
    *ptr = 0;
    pthread_exit((void *) ptr);
}


Boolean check_lettura (struct mailbox_t *mb){
    Boolean  ok = true;
    for (int i = 0; i < 3; i++) {
        if (mb->conferma_lettura[i] != 1) {
            printf("[CHECK FUNC] HO TROVATO UNO CHE NO HA LETTO [%d] [%d] [%d]\n",mb->conferma_lettura[i]);
            ok = false;
        }
    }
    printf("[CHECK FUNC]  HANNO LETTO TUTTI [%d] [%d] [%d]\n",mb->conferma_lettura[0],mb->conferma_lettura[1],mb->conferma_lettura[2]);
    return ok;
}


void leggi (struct mailbox_t *mb, int *pi){
    T msg;
    Boolean letto;

    while (&mb->n_msg <= 0)
        pthread_cond_wait(&mb->piena,&mb->mtx);

    //leggo

    //aspetto la notifica di un nuovo messaggio
    pthread_cond_wait(&mb->nuovo_msg,&mb->mtx);

    msg = mb->coda_circolare[mb->tail];
    printf("[RECEIVER %d]\t\t Ho letto il messaggio\t%d dalla posizione\t[%d]\n",*pi,msg,mb->tail);

    //segno anche che ho letto
    //printf("\t\t\t\tSTO PER METTERE UN 1 IN POSIZIONE %d\n",*pi);
    mb->conferma_lettura[*pi] = 1;
    printf("\t\t\t\t[SITUAZIONE] %d\t%d\t%d\n",mb->conferma_lettura[0],mb->conferma_lettura[1],mb->conferma_lettura[2]);

    letto = check_lettura(mb);
    if (check_lettura(mb) != true){
        printf("[RECEIVER %d] MI BLOCCO \n",*pi);
        pthread_cond_wait(&mb->letto_da_tutti,&mb->mtx);
        printf("[RECEIVER %d] MI SSBLOCCO \n",*pi);
    } else {
        pthread_cond_broadcast(&mb->letto_da_tutti);
        mb->tail = (mb->tail + 1) % N;
        mb->n_msg --;
        for (int i = 0; i < 3; i++) mb->conferma_lettura[i] = 0;
    }
}

void ricevi (struct mailbox_t *mb, int *pi){
    //controllo se c' e' qualcosa da leggere
    int * indice = (int*) pi;

    printf("ECCO IL MIO INDICE \t\t\t\t\t%d\n",*indice );

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

    taskids[0] = 0; taskids[1] = 1;taskids[2] = 2;taskids[3] = 3;
    pthread_create(&thread[0], NULL, receiver, (void *) (&taskids[0]));
    pthread_create(&thread[1], NULL, receiver, (void *) (&taskids[1]));
    pthread_create(&thread[2], NULL, receiver, (void *) (&taskids[2]));
    pthread_create(&thread[3], NULL, generatore, (void *) (&taskids[3]));

    sleep(2);

    /*for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, receiver, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }*/

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




