#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define  NTIMES 20                                      // numero di letture da effettuare

#define N 10                                            //dimensione coda
#define THREAD 4                                        //Numero di Threads --> 3 ricevitori e un generatore

typedef enum {false, true} Boolean;
typedef int T;                                          //definisco il tipo T dei messaggi che e' un intero

int valGlobale;                                         //variabile che sara' il contenuto dei messaggi

//struttura mailbox
struct mailbox_t {
    pthread_cond_t vuota,piena,letto_da_tutti;          //2 cond var per il controllo sulla mailbox ed una per aspettare la lettura di tutti
    pthread_cond_t synch[3];                            //3 cond var per sincronizzare l'ordine di lettura dei ricevitori
    pthread_mutex_t mtx;                                //mutex
    int coda_circolare[N];                              //array circolare dove memorizzare i messaggi
    int head, tail;                                     //head e tail per gestire inserimento e prelievo dalla mailbox
    int n_msg;                                          //variabile che indica quanti messaggi sono presenti nella mailbox (0 <= n_msg <= N)
    int conferma_lettura[3];                            //array binario dove il ricevitore i-esimo, una volta letto il messaggio setta un 1
    int turno;                                          //indica il turno del ricevitore che deve leggere
}mailbox;


//inizializzazione struttura
void init_mailbox(struct mailbox_t *mb){

    //inizializzazione cond var
    pthread_cond_init(&mb->vuota, NULL);
    pthread_cond_init(&mb->piena, NULL);
    pthread_cond_init(&mb->letto_da_tutti, NULL);

    //initzializzazione mutex
    pthread_mutex_init(&mb->mtx, NULL);

    //inizializazione coda circolare
    mb->head = mb->tail = 0;
    for (int j = 0; j < N; ++j) mb->coda_circolare[j] = -1;
    //inizializzazione conferma lettura e cond var synch
    for (int i = 0; i < 3; ++i) {
        mb->conferma_lettura[i] = 0;
        pthread_cond_init(&mb->synch[i],NULL);
    }
    //inizializzo numero messaggi a 0 e turno a 0
    mb->n_msg = 0;
    mb->turno = 0;
}

void genera_msg(struct mailbox_t *mb){
    //controllo se c'e' posto nella mailbox
    pthread_mutex_lock(&mb->mtx);

    while (mb->n_msg >= N){                                         //se la mailbox e' piena...
        printf("[SERVER]\t\tMAILBOX PIENA\n");
        pthread_cond_wait(&mb->vuota ,&mb->mtx);                    //...aspetto
    }

    //posso generare il msg
    T msg = valGlobale;
    valGlobale ++;                                                  //incremento valGlobale
    mb->coda_circolare[mb->head] = msg;                             //inserisco il messaggio nella mailbox
    printf("\n\n[SERVER]\t\tHO APPENA GENERATO IL MESSAGGIO '%d'E MESSO IN MAILBOX[%d]\n",msg,mb->head);
    mb->head = (mb->head + 1) % N;                                  //aggiorno la head
    mb->n_msg ++;                                                   //e il numero di messaggio nella mailbox

    pthread_cond_broadcast(&mb->piena);                             //sveglio eventuali ricevitori bloccati a causa di mailbox vuota

    pthread_mutex_unlock(&mb->mtx);
}

void *generatore (void * id){
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    while(1){
        genera_msg(&mailbox);                                //genero un messaggio
        sleep(rand()%3);                                    //aspetto qualche secondo
    }

    //non arrivero' mai qui
   //*ptr = 0;
   // pthread_exit((void *) ptr);
}


Boolean check_lettura (struct mailbox_t *mb){
    //controlla se tutti e tre i ricevitori hanno letto
    Boolean  ok = true;                                         //setto ok a true...
    for (int i = 0; i < 3; i++) {
        if (mb->conferma_lettura[i] != 1) {                     //se ne trovo uno che non ha ancora letto...
            printf("[CHECK FUNC]\t\t%d --> Non ha ancora letto \n",i);
            ok = false;                                         //...setto ok a false
        }
    }
    return ok;
}


void leggi (struct mailbox_t *mb, int *pi){
    T msg;
    //leggo il messaggio dalla mailbox e lo salvo in msg
    msg = mb->coda_circolare[mb->tail];
    printf("[RECEIVER %d]\t\tHO LETTO IL MESSAGGIO\t%d DALLA POSIZIONE\t[%d]\n",*pi,msg,mb->tail);

    //segno anche che ho letto
    mb->conferma_lettura[(int)*pi] = 1;
    printf("\t\t\t[SITUAZIONE]\t%d\t%d\t%d\n",mb->conferma_lettura[0],mb->conferma_lettura[1],mb->conferma_lettura[2]);

    //incremento il turno...
    if (mb->turno != 2)
        mb->turno ++;
    else mb->turno = 0;

    //..e sveglio il processo che deve leggere
    pthread_cond_signal(&mb->synch[mb->turno]);

    //controllo se hanno letto tutti
    if (check_lettura(mb)){
        pthread_cond_broadcast(&mb->letto_da_tutti);                            //se si, sveglio tutti i ricevitori bloccati in attesa del completamento del ciclo di lettura
        printf("[CHECK FUNC]\t\tHanno letto tutti [%d] [%d] [%d]\n",mb->conferma_lettura[0],mb->conferma_lettura[1],mb->conferma_lettura[2]);
        for (int i = 0; i < 3; ++i) mb->conferma_lettura [i] = 0;               //azzero i valori dell'array conferma_lettura
        mb->tail = (mb->tail + 1) % N;                                          //aggiorno la tail
        mb->n_msg --;                                                           //decremento il num di msg nella mailbox
        pthread_cond_signal(&mb->vuota);                                        //se il processo generatore e' bloccato per mailbox piena, lo sveglio
    } else
        pthread_cond_wait(&mb->letto_da_tutti,&mb->mtx);                        //altrimenti mi blocco
}

void ricevi (struct mailbox_t *mb, int *pi){
    //controllo prima se c' e' qualcosa da leggere
    int * indice = (int*) pi;
    pthread_mutex_lock(&mb->mtx);

    //se la mailbox e' vuota...
    while (mb->n_msg <= 0){
        printf("[RECEIVER %d]\t\tMAILBOX VUOTA\n",*pi);
        pthread_cond_wait(&mb->piena,&mb->mtx);                                     //..mi blocco
    }

    //controllo se il mio indice puo leggere o deve aspettare che leggano prima gli altri
    while(mb->turno != *pi){
        printf("[RECEIVER %d]\t\tTURNO %d\n",*pi,mb->turno);
        pthread_cond_signal(&mb->synch[mb->turno]);                                //se non e' il mio turno, sveglio il ricevitore giusto...
        pthread_cond_wait(&mb->synch[*pi],&mb->mtx);                               //..e mi blocco
    }

    //e' il mio turno.. posso leggere
    printf("[RECEIVER %d]\t\tTURNO %d POSSO LEGGERE\n",*pi,mb->turno);
    leggi(&mailbox,indice);

    pthread_mutex_unlock(&mb->mtx);
}

void *receiver (void *id) {
    int *pi = (int *)id;
    int letture = 0;                                            //ogni ricevitore segna il numero di letture effettuate
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (int t = 0; t < NTIMES; t++ ){
        ricevi(&mailbox,pi);                                    //leggo
        sleep(1);
        letture ++;                                             //incremento le letture
    }
   *ptr = letture;
   // pthread_exit((void *) ptr);                                 //ritorno il numero di letture effettuate
}



int main (int argc, char **argv){
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS = THREAD;
    char error[250];

    //inizializziamo la struttura
    init_mailbox(&mailbox);

    //allocazione memoria per i thread
    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL){
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }

    //allocazione mermoria per taskids
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL){
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    //inizializzazione seme per rand()
    srand(time(NULL));

    //creazione thread
    for (i=0; i < NUM_THREADS; i++) {
        taskids[i] = i;
        if (i == NUM_THREADS - 1) {             //l'ultimo thread e' il generatore
            if (pthread_create(&thread[i], NULL, generatore, (void *) (&taskids[i])) != 0) {
                sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",taskids[i]);
                perror(error);
                exit(5);
            }
        } else {
            if (pthread_create(&thread[i], NULL, receiver, (void *) (&taskids[i])) != 0) {
                sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",taskids[i]);
                perror(error);
                exit(5);
            }
        }
    }

    for (i=0; i < NUM_THREADS-1; i++){
        int ris;
        // attendiamo la terminazione di tutti i thread generati
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d <-- numero di letture effettuate\n", i, ris);
    }

    exit(0);
}
