// aggiusta la terminazione e altri piccoli dettagli
//Passa 2 parametri M mittenti R ricevitori
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 10 // dimensione coda
#define NTIMES 10

typedef enum {false, true} Boolean;
typedef int T;

int valGlobale;
int R,M;

struct mailbox_t {
    int* coda_circolare;
    int head , tail;
    int n_msg;
    pthread_cond_t vuota,piena;
    pthread_mutex_t mtx;
    pthread_cond_t *synch;
    int turno;
    int bloccati_su_vuota, bloccati_su_piena;

}mailbox;


void init_mailbox(struct mailbox_t *mb){

    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);

    pthread_mutex_init(&mb->mtx, NULL);
    mb->head = mb->tail = 0;
    mb->coda_circolare = malloc(N * sizeof (int*));
    for (int j = 0; j < N; ++j) mb->coda_circolare[j] = (int*)-1;
    mb->synch = malloc(R * sizeof(pthread_cond_t));
    for (int i = 0; i < R; i++) pthread_cond_init(&mb->synch[i],NULL);
    mb->n_msg = 0;
    mb->turno = 0;
    mb->bloccati_su_piena = mb->bloccati_su_vuota = 0;

}

void send1(struct mailbox_t *mb){
    //controllo se c'e' posto nella mailbox

    pthread_mutex_lock(&mb->mtx);
    while (mb->n_msg >= N){
        //aspetto perche' la mailbox e' piena
        printf("[SERVER]\t\tMAILBOX PIENA\n");
        mb->bloccati_su_piena ++;
        pthread_cond_wait(&mb->vuota ,&mb->mtx);
        mb->bloccati_su_piena --;
    }
    //posso generare il msg
    T msg = valGlobale;
    valGlobale ++;
    mb->coda_circolare[mb->head] = msg;
    mb->head = (mb->head + 1) % N;
    mb->n_msg ++;
    printf("\n\n[SERVER SEND 1]\t\tHO APPENA GENERATO IL MESSAGGIO '%d'E MESSO IN MAILBOX[%d]\n",msg,mb->head);

    // introduci int blccati_su_vuota
    if (mb->bloccati_su_vuota > 0) pthread_cond_broadcast(&mb->piena);

    pthread_mutex_unlock(&mb->mtx);
    sleep(2);
}

void send2(struct mailbox_t *mb){
    //controllo se c'e' posto nella mailbox

    pthread_mutex_lock(&mb->mtx);
    while (mb->n_msg > N - 2){          // la send2 deve inserire 2 msg. Controllo quindi che ci siano 2 posti liberi per continuare
        //aspetto perche' la mailbox e' piena
        printf("[SERVER]\t\tMAILBOX PIENA\n");
        mb->bloccati_su_piena ++;
        pthread_cond_wait(&mb->vuota ,&mb->mtx);
        mb->bloccati_su_piena --;
    }
    //posso generare i 2 msg
    for (int t = 0; t < 2; ++t) {
        T msg = valGlobale;
        valGlobale ++;
        mb->coda_circolare[mb->head] = msg;
        mb->head = (mb->head + 1) % N;
        mb->n_msg ++;
        printf("\n\n[SERVER SEND2]\t\tHO APPENA GENERATO IL %d MESSAGGIO: '%d'E MESSO IN MAILBOX[%d]\n",t+1,msg,mb->head);

    }

    if (mb->bloccati_su_vuota > 0) pthread_cond_broadcast(&mb->piena); // notifico a tutti la presenza di un nuovo messaggio
    pthread_mutex_unlock(&mb->mtx);
    sleep(2);
}

void *mittente (void * id){
    int *pi = (int *)id;
    int *ptr;
    int invii = 0;
    ptr = (int *) malloc( sizeof(int));
    int proc = rand() % 2;
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    for (int t = 0; t < NTIMES; t ++ ){
        if (proc)
            send1(&mailbox);
        else
            send2(&mailbox);

        sleep(3);
        invii ++;
    }
    *ptr = invii;
    pthread_exit((void *) ptr);
}


void leggi (struct mailbox_t *mb, int *pi){
    T msg;
    //leggo

    msg = mb->coda_circolare[mb->tail];
    printf("[RECEIVER %d]\t\t HO LETTO IL MESSAGGIO\t%d DALLA POSIZIONE\t[%d]\n",*pi,msg,mb->tail);

    // aggiorno i dati nella mailbox
    mb->tail = (mb->tail + 1) % N;
    mb->n_msg --;
    pthread_cond_signal(&mb->vuota);

    //devo svegliare i processi bloccati per l'indice
    mb->turno = (mb->turno + 1) % R;

    pthread_cond_signal(&mb->synch[mb->turno]);
    //printf("[RECEIVER %d]\t\tRiparto \n",*pi);
}

void ricevi (struct mailbox_t *mb, int *pi){
    int * indice = (int*) pi;

    pthread_mutex_lock(&mb->mtx);

    //controllo se c' e' qualcosa da leggere
    while (mb->n_msg <= 0){
        printf("[RECEIVER %d qui] MAILBOX VUOTA\n",*pi);
        mb->bloccati_su_vuota ++;
        pthread_cond_wait(&mb->piena,&mb->mtx);
        mb->bloccati_su_vuota --;
    }
    //controllo se il mio indice puo leggere o deve aspettare che leggano prima gli altri
    while(mb->turno != *pi){
        printf("[RECEIVER %d] TURNO %d\n",*pi,mb->turno);
        pthread_cond_signal(&mb->synch[mb->turno]);
        printf("[RECEIVER %d] TURNO %d MI BLOCCO\n",*pi,mb->turno);
        pthread_cond_wait(&mb->synch[*pi],&mb->mtx);
    }

    printf("[RECEIVER %d] TURNO %d MI SBLOCCO\n",*pi,mb->turno);
    leggi(&mailbox,indice);

    pthread_mutex_unlock(&mb->mtx);
}

void *receiver (void *id) {
    int *pi = (int *)id;
    int *ptr;
    int letture = 0;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (int t = 0; t < NTIMES + 10; t++){
        // printf("ECCO IL MIO INDICE \t%d\n",*pi);
        ricevi(&mailbox,pi);
        sleep(3);
        letture ++;
    }
    *ptr = letture;
    pthread_exit((void *) ptr);
}



int main (int argc, char **argv)
{
    pthread_t *threadM, *threadR;
    int *taskidsR, *taskidsM;
    int i;
    int *p;
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

