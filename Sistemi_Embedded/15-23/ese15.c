//
// Created by davide on 03/01/22.
//
/**
 * Simulare il comportamento di conto corrente bancario sul quale piu` utenti (processi) possono effettuare
prelievi e/o depositi, con il vincolo che il conto non possa mai andare in rosso (il totale depositato deve
essere sempre maggiore o uguale a zero. Utilizzare il costrutto monitor per simulare il conto corrente, con
due procedure: deposito e prelievo..
Evitare di svegliare un processo quando non si Ë sicuri che possa ripartire.
Fornire due soluzioni: una che prevede che un processo non sia bloccato quando le condizioni logiche per
la sua esecuzione sono verificate ed una che prevede che i prelievi siano serviti rigorosamente FIFO,
cioË che un processo che puÚ essere servirto si blocchi se qualcuno ha richiesto un prelievo prima di lui
che non puÚ essere servito
 */

// incompleto --> non garantisce ordine fifo

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

int NUM_THREADS;

typedef enum {false, true} Boolean;

pthread_cond_t *stop,ok_prelievo;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
long int saldo;
int prelievi_blocc;
int *lista_prelievi_b;

int *coda_c;
int head, tail;

int NUM_THREADS;


void myInit(){
    stop = malloc( NUM_THREADS * (sizeof (pthread_cond_t)));
    coda_c = malloc(NUM_THREADS * sizeof (int));
    lista_prelievi_b = malloc(NUM_THREADS * sizeof (int));

    for (int i = 0; i < NUM_THREADS; i++) {
        lista_prelievi_b[i] = -1;
        coda_c[i] = -1;
        pthread_cond_init(&stop[i],NULL);
    }
    pthread_cond_init(&ok_prelievo,NULL);
    saldo = 1000;
    prelievi_blocc = 0;
    head = tail = 0;
}

void deposito(int *pi , int somma){
    pthread_mutex_lock( &m);
    printf("\n[UTENTE %d]\t\tVUOLE EFFETTUARE UN DEPOSITO DI %d\n",*pi,somma);
    saldo = saldo + somma;
    printf("[UTENTE %d]\t\tVUOLE HA DEPOSITATO %d EURO. NUOVO SADO %d\n",*pi,somma,saldo);
    sleep(1);
    pthread_mutex_unlock(&m);
    pthread_cond_signal(&stop[tail]);
}

void bloccami (int *pi, int somma){
    while (tail != *pi){
        if (saldo - lista_prelievi_b[tail] > 0) {
            //prelievi_blocc--;
            printf("[UTENTE %d] SVEGLIATO UTENTE %d BLOCCATO \n", *pi, coda_c[tail]);
            pthread_cond_signal(&stop[tail]);
        }
        /*} else {
            pthread_cond_wait(&stop[head],&m);
        }*/
        tail = (tail + 1) % NUM_THREADS;
    }
}


void preleva (int *pi, int somma){
    pthread_mutex_lock( &m);
    printf("\n[UTENTE %d] VUOLE PRELEVARE %d \n",*pi,somma);

    // controllo prima se ci sono processi bloccati su preleva
    coda_c[head] = *pi;
    lista_prelievi_b [head]= somma;
    head = (head + 1) % NUM_THREADS;

    if (prelievi_blocc > 0) bloccami(pi,somma);

    while ( saldo - somma < 0){
        printf("[UTENTE %d] BLOCCATO SU PRELEVA %d X SALDO INSUFFICENTE: %d\n",*pi,somma,saldo);
        prelievi_blocc ++;
        pthread_cond_wait(&stop[head],&m);
        prelievi_blocc --;
        printf("[UTENTE %d] SVEGLIATO  %d\n",*pi,prelievi_blocc);
    }

    saldo -= somma;
    printf("[UTENTE %d] PRELEVATO %d -->  SALDO : %d\n",*pi,somma,saldo);

    pthread_mutex_unlock(&m);
}


void *eseguiCliente(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int r,index;
    //int posti_occupati = 0;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //niente ciclo per ora, si consiglia invocazione con numero grande... es 100
    for (;;) {
        r = rand() % 2;     // 0 prelivo 1 deposito
        if (r)
            deposito(pi,(rand()%600) + 1 );
        else
            preleva (pi,(rand()%800) + 1);
        sleep(2);
    }



    pthread_mutex_unlock(&m);
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv)
{
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
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

    // addetto
    sleep(1);

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiCliente, (void *) (&taskids[i])) != 0)
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
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    pthread_mutex_destroy(&m);
    exit(0);
}



