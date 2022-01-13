
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 10 // dimensione coda

typedef enum {false, true} Boolean;
typedef int T;

int valGlobale;

struct mailbox_t {
    int* coda_circolare;
    int head , tail;
    int n_msg;
    pthread_cond_t vuota,piena,letto_da_tutti,stop;
    pthread_mutex_t mtx;
    int *conferma_lettura[3];
    pthread_cond_t synch[3];
    int turno;
}mailbox;

Boolean check_lettura (struct mailbox_t *mb);

void init_mailbox(struct mailbox_t *mb){

    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);
    pthread_cond_init(&mb->letto_da_tutti, NULL);
    pthread_cond_init(&mb->stop, NULL);

    pthread_mutex_init(&mb->mtx, NULL);
    mb->head = mb->tail = 0;
    mb->coda_circolare = malloc(N * sizeof (int*));
    for (int j = 0; j < N; ++j) mb->coda_circolare[j] = (int*)-1;
    for (int i = 0; i < 3; ++i) {
        mb->conferma_lettura[i] = (int*)0;
        pthread_cond_init(&mb->synch[i],NULL);
    }
    mb->n_msg = 0;
    mb->turno = 0;
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
    printf("\n\n[SERVER]\t\tHO APPENA GENERATO IL MESSAGGIO '%d'E MESSO IN MAILBOX[%d]\n",msg,mb->head);
    mb->head = (mb->head + 1) % N;
    mb->n_msg ++;

    // introduci int blccati_su_vuota
    pthread_cond_broadcast(&mb->piena);

    //pthread_cond_broadcast(&mb->letto_da_tutti);
    pthread_mutex_unlock(&mb->mtx);
    sleep(4);
    //ora spetto che tutti e 3 i receivers abbiano letto il msg
    pthread_mutex_lock(&mb->mtx);

    while(!check_lettura(&mailbox))
        pthread_cond_wait(&mb->letto_da_tutti,&mb->mtx);

    for (int i = 0; i < 3; ++i) mb->conferma_lettura [i] = (int*)0;
    mb->tail = (mb->tail + 1) % N;
    mb->n_msg --;
    pthread_mutex_unlock(&mb->mtx);
    pthread_cond_broadcast(&mb->stop);

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
        genera_msg(&mailbox);
        //sleep(1);
    }
    *ptr = 0;
    pthread_exit((void *) ptr);
}


Boolean check_lettura (struct mailbox_t *mb){
    Boolean  ok = true;
    for (int i = 0; i < 3; i++) {
        if (mb->conferma_lettura[i] != 1) {
            printf("[CHECK FUNC]\t\t%d --> Non ha ancora letto \n",i);
            ok = false;
        }
    }
    return ok;
}


void leggi (struct mailbox_t *mb, int *pi){
    T msg;
    Boolean letto;
    //leggo

    msg = mb->coda_circolare[mb->tail];
    printf("[RECEIVER %d]\t\t HO LETTO IL MESSAGGIO\t%d DALLA POSIZIONE\t[%d]\n",*pi,msg,mb->tail);

    //segno anche che ho letto
    mb->conferma_lettura[(int)*pi] = (int*)1;
    printf("\t\t\t\t[SITUAZIONE] %d\t%d\t%d\n",mb->conferma_lettura[0],mb->conferma_lettura[1],mb->conferma_lettura[2]);

    if (check_lettura(mb)){
        pthread_cond_signal(&mb->letto_da_tutti);
        printf("[CHECK FUNC]  Hanno letto tutti [%d] [%d] [%d]\n",mb->conferma_lettura[0],mb->conferma_lettura[1],mb->conferma_lettura[2]);

    }
    //printf("[RECEIVER %d] MI SBLOCCO \n",*pi);
    pthread_cond_signal(&mb->vuota);
    //prima di bloccarmi devo svegliare i processi bloccati per l'indice
    if (mb->turno != 2)
        mb->turno ++;
    else mb->turno = 0;

    pthread_cond_signal(&mb->synch[mb->turno]);
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
    //controllo se il mio indice puo leggere o deve aspettare che leggano prima gli altri
    while(mb->turno != *pi){

        printf("[RECEIVER %d] TURNO %d\n",*pi,mb->turno);

        pthread_cond_signal(&mb->synch[mb->turno]);
        pthread_cond_wait(&mb->synch[*pi],&mb->mtx);
        printf("[RECEIVER %d] TURNO %d ME BLOCCO\n",*pi,mb->turno);

    }

    printf("[RECEIVER %d] TURNO %d ME SBLOCCO\n",*pi,mb->turno);
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
