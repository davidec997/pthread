//
//Passa 2 parametri M mittenti R ricevitori
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
    pthread_cond_t vuota,piena;
    pthread_mutex_t mtx;
    int turno;

}mailbox;

Boolean check_lettura (struct mailbox_t *mb);

void init_mailbox(struct mailbox_t *mb){

    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);

    pthread_mutex_init(&mb->mtx, NULL);
    mb->head = mb->tail = 0;
    mb->coda_circolare = malloc(N * sizeof (int*));
    for (int j = 0; j < N; ++j) mb->coda_circolare[j] = (int*)-1;
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
    mb->head = (mb->head + 1) % N;
    mb->n_msg ++;
    printf("\n\n[SERVER]\t\tHO APPENA GENERATO IL MESSAGGIO '%d'E MESSO IN MAILBOX[%d]\n",msg,mb->head);

    // introduci int blccati_su_vuota
    pthread_cond_broadcast(&mb->piena);

    //pthread_cond_broadcast(&mb->letto_da_tutti);
    pthread_mutex_unlock(&mb->mtx);
    sleep(4);

    //ora spetto che tutti e 3 i receivers abbiano letto il msg

    pthread_mutex_unlock(&mb->mtx);
    //pthread_cond_broadcast(&mb->stop);

}

void *mittente (void * id){
    int *pi = (int *)id;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    int proc = rand() % 2;
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    while(1){
        if (proc){
            genera_msg(&mailbox);
        }
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

    //segno anche che ho letto


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
    pthread_t *threadM, *threadR;
    int *taskidsR, *taskidsM;
    int i;
    int *p;
    int R,M;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 3 ) /* Deve essere passato esattamente un parametro */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    M = atoi(argv[1]);
    R = atoi(argv[2]);

    if (M <= 0 || R <= 0)
    {
        sprintf(error,"Errore: Il primo o secondo parametro non e' un numero strettamente maggiore di 0 ma e' %d %d\n",M, R);
        perror(error);
        exit(2);
    }

    init_mailbox(&mailbox);

    threadM=(pthread_t *) malloc(M * sizeof(pthread_t));
    threadR=(pthread_t *) malloc(R * sizeof(pthread_t));
    if (threadM == NULL || threadR == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskidsM = (int *) malloc(M * sizeof(int));
    taskidsR = (int *) malloc(R * sizeof(int));

    if (taskidsM == NULL || taskidsR == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    srand(555);

    sleep(1);
    for (i=0; i < M; i++) {
        taskidsM[i] = i;
        if (pthread_create(&threadM[i], NULL, mittente, (void *) (&taskidsM[i])) != 0) {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",taskidsM[i]);
            perror(error);
            exit(5);
        }
    }

    for (i=0; i < R; i++) {
        taskidsR[i] = i;

        if (pthread_create(&threadR[i], NULL, receiver, (void *) (&taskidsR[i])) != 0) {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",
                    taskidsR[i]);
            perror(error);
            exit(5);
        }
    }

    for (i=0; i < M; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(threadM[i], (void**) & p);
        ris= *p;
        printf("MITTENTE %d-esimo restituisce %d\n", i, ris);
    }
    for (i=0; i < R; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(threadR[i], (void**) & p);
        ris= *p;
        printf("RICEVENTE %d-esimo restituisce %d\n", i, ris);
    }

    exit(0);
}

