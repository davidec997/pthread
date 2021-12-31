//
// Created by davide on 30/12/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NTIMES 30

 // dimensione coda
#define  S 5

typedef enum {false, true} Boolean;
char *senders [5] = {"SENDER A", "SENDER B", "SENDER C","SENDER D", "SENDER E"};
typedef int T;

int valGlobale;
struct mailbox_t {
    int* coda_circolare;
    int head , tail;
    int n_msg;
    pthread_cond_t vuota,piena,reset, *synch,gruppo,stop;
    pthread_mutex_t mtx;
    Boolean mb_ok;
    int turno;
    int grp;
}mailbox;


void init_mailbox(struct mailbox_t *mb){

    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);
    pthread_cond_init(&mb->reset, NULL);
    pthread_cond_init(&mb->stop, NULL);


    mb->synch= malloc(S*sizeof (pthread_cond_t));

    pthread_mutex_init(&mb->mtx, NULL);
    pthread_cond_init(&mb->gruppo, NULL);

    mb->head = mb->tail = 0;
    mb->coda_circolare = malloc(S * sizeof (int*));
    for (int j = 0; j < S; ++j){
        mb->coda_circolare[j] = (int*)-1;
        pthread_cond_init(&mb->synch[j],NULL);
    }
    mb->n_msg = 0;
    mb->turno = 0;
    mb->mb_ok = false;
    mb->grp = -1;

}


void genera_msg(struct mailbox_t *mb, int *pi){

    pthread_mutex_lock(&mb->mtx);
    int mygroup;
    if (*pi > 9)
        mygroup = 2;
    else
        mygroup = 1;

    // VERIFICO SE SONO IL PRIMO AD ENTRARE
    if (mb->turno == 0 ) {
        // sono il primo e nessuno ha ancora scritto sulla mb
        //setto il gruppo
        // INDICI DA 0 A 4 <-- GRUPPO A    INDICI DA 10 A 14 GRUPPO B
        if (*pi > 9) {
            mb->grp = 2;
        } else {
            mb->grp = 1;
        }
        printf("[SENDER %d] HO SETTATO IL GRUPPO: %d\n",*pi,mb->grp);
    }

    while (mb->grp != mygroup){
        printf("[SENDER %d] GRUPPO %d\t\tMI BLOCCO XK IL GRUPPO ATTUALE E' : %d\n",*pi,mygroup,mb->grp);
        pthread_cond_wait(&mb->gruppo,&mb->mtx);
    }

    //controllo se e' il mio turno
    // DEVI MODIFICARE IL TURNO CHE ORA PUO ARRIVARE FINO A 14
    while(mb->turno != *pi) {
        printf("[SENDER %d] TURNO %d\n",*pi,mb->turno);
        pthread_cond_signal(&mb->synch[mb->turno]);
        printf("[SENDER %d] TURNO %d MI BLOCCO\n",*pi,mb->turno);
        pthread_cond_wait(&mb->synch[*pi],&mb->mtx);
    }


    //controllo se c'e' posto nella mailbox
    while (mb->n_msg >=S){
        //aspetto perche' la mailbox e' piena
        printf("[SERVER] MAILBOX PIENA\n");
        pthread_cond_wait(&mb->vuota ,&mb->mtx);
    }
    //posso generare il msg
    T msg = rand() % 20;
    //valGlobale ++;
    mb->coda_circolare[mb->head] = msg;
    printf("[SENDER %d]\t\tHo appena generato il msg %d e messo in mb[%d]\n",*pi,msg,mb->head);
    mb->head = (mb->head + 1) % S;
    mb->n_msg ++;

    if (mb->n_msg == S)  {
        pthread_cond_signal(&mb->piena);
        mb->mb_ok = true;
        mb->turno = 0;
    } else
        mb->turno = (mb->turno + 1) % S;

    pthread_cond_signal(&mb->synch[mb->turno]);
    //pthread_cond_wait(&mb->stop,&mb->mtx);
    pthread_mutex_unlock(&mb->mtx);

    //devo svegliare i processi bloccati per l'indice
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
        genera_msg(&mailbox,pi);
    }
    *ptr = 0;
    pthread_exit((void *) ptr);
}

void resetta (struct mailbox_t *mb){

    pthread_mutex_lock(&mb->mtx);
    mb->tail = 0;
    mb->head = 0;
    mb->n_msg = 0;
    mb->mb_ok = false;
    mb->turno = 0;
    pthread_mutex_unlock(&mb->mtx);
    pthread_cond_signal(&mb->synch[mb->turno]);


}


void leggi (struct mailbox_t *mb, int *pi){
    T msg;
    Boolean letto;
    //leggo
    //pthread_cond_wait(&mb->piena,&mb->mtx);

    // svuota la mb

    for (int i = 0; i < S; i++) {
        msg = mb->coda_circolare[mb->tail];
        printf("[RECEIVER %d]\t\t Ho letto il messaggio\t%d dalla posizione\t[%d]\n",*pi,msg,mb->tail);
        mb->tail = (mb->tail + 1) % S;
        //mb->n_msg --;
    }

    printf("[RECEIVER %d]\t\t HO SVUOTATO LA MAILBOX.. NUOVO GIRO\n",*pi,msg,mb->tail);

    //resetta(mb);
    mb->mb_ok = false;
    mb->n_msg = 0;

    pthread_cond_signal(&mb->vuota);
    //mb->grp = -1;
    //pthread_cond_broadcast(&mb->stop);
    if (mb->grp == 1)
        mb->grp = 2;
    else
        mb->grp = 1;

    pthread_cond_broadcast(&mb->gruppo);

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
    pthread_t *thread,*th;
    int *taskids,*td2;
    int i;
    int *p;
    int NUM_THREADS = 6;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 2 ) /* Deve essere passato esattamente un parametro */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    //NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0)
    {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
    }

    init_mailbox(&mailbox);

    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    th=(pthread_t *) malloc((NUM_THREADS-1) * sizeof(pthread_t));

    if (thread == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    td2 = (int *) malloc((NUM_THREADS-1) * sizeof(int));
    if (taskids == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }


    srand(555);

    //sleep(2);

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        if (i == NUM_THREADS -1){
            pthread_create(&thread[i], NULL, receiver, (void *) (&taskids[i]));
        } else {
            if (pthread_create(&thread[i], NULL, generatore, (void *) (&taskids[i])) != 0)
            {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(5);
            }
        }

        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=0; i < NUM_THREADS-1; i++){
        td2[i] = i+10;
        pthread_create(&th[i], NULL, generatore, (void *) (&td2[i]));
    }

/*
        pthread_create(&th, NULL, generatore, (void*)10);
    pthread_create(&th, NULL, generatore, (void*)11);
    pthread_create(&th, NULL, generatore, (void*)12);
    pthread_create(&th, NULL, generatore, (void*)13);
    pthread_create(&th, NULL, generatore, (void*)14);*/


    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);


    exit(0);
}




