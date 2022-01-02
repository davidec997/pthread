//
// Created by dada on 02/01/22.
//
/*
In un sistema a memoria comune quattro processi applicativi P1, P2, P3, P4 competono per l'uso di due
risorse equivalenti. Si chiede di scrivere il codice del gestore per allocare dinamicamente le risorse ai
richiedenti tenendo conto che, all'atto della richiesta, il richiedente specifica anche un parametro
T:integer che denota un timeout (in termini di tick di orologio) scaduto il quale, se la richiesta non è stata
esaudita il processo richiedente viene comunque svegliato pur non avendo disponibile la risorsa richiesta.
identificate le procedure del gestore con i necessari parametri, scrivere il codice del gestore supponendo
che ad ogni tick di orologio vada in esecuzione il relativo processo orologio. Se necessaria, è disponibile
        la primitiva di sistema PE che restituisce l'indice del processo in esecuzione. Non è specificata nessuna
politica per quanto riguarda la priorità dei processi richiedenti.
*/

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 2 // risorse
#define ITER 6
typedef enum {false, true} Boolean;
//araray bool per le risrse
Boolean libero [2];
sem_t m, risorse[2], t,go;
int time_o [4];
int chiamante;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&t,0,0);
    sem_init(&go,0,0);


    sem_init(&risorse[0],0,0);
    sem_init(&risorse[1],0,0);

    for (int i = 0; i < 2; i++) libero[i]=true;

    for (int f = 0; f < 4; ++f){
        time_o[f] = -1;
    }
    chiamante = -1;
}


int richiesta( int timeout, int risorsa ,int *pi){
    // far partire il timer
    sem_wait(&m);
    time_o[*pi] = timeout;
     if (chiamante == -1)
         chiamante = *pi;

    sem_post(&m);
    sem_post(&t);
    sem_wait(&go);

    // qui mi sono svegliato
    // o mi sono svegliato xk la risorsa e' libera oppure mi ha svegliato il timer

    sem_wait(&m);

    if (libero[risorsa] == true){
        // allora la ris e' libera
        libero[risorsa]= false;
        sem_post(&risorse[risorsa]); // post previa
        sem_wait(&risorse[risorsa]);
        printf("[PROCESSO %d] LA RISORSA %d  E' LIBERA.. LA USO\n",*pi, risorsa);

    } else {
        printf("[PROCESSO %d] LA RISORSA %d NON E' LIBERA.. QUITTO\n",*pi, risorsa);
    }

    sem_post(&m);



    /*sem_wait(&risorse);
    sem_wait(&m);
    int i;
    for (i = 0; !libero[i]; ++i);
    libero[i]= false;
    sem_post(&m);
    return i;*/
}


void rilascio(int risorsa, int *pi){
    sem_wait(&m);
    libero[risorsa]= true;
    time_o[*pi] = -1;
    sem_post(&m);
    //sem_post(&risorse);
}

void oro ( int *pi , int timeout, int risorsa){

}

void *orologio( void  * id){
    int *pi = (int *) id;
    int *ptr =0;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    while(1){
        sem_wait(&t);
        sem_wait(&m);
        for (int i = 0; i <= time_o[chiamante]; i++) {
            printf("[TIMER]\t\tITER %d del TIMERE PER PROCESSO %d\n",i,chiamante);
            sleep(1);
        }


    }

    pthread_exit((void *) ptr);

}

void *esegui(void *id) {
    int *pi = (int *) id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    int x; //risorsa
    printf("sto per iniziare thread %d\n",(int*)pi);
    for (int t = 0; t < ITER; t++){
        x = richiesta(rand()% 4);
        printf("Il thread %d ha OTTENUTO  la risorsa \t[%d]\n",*pi, x);
        sleep(4);
        rilascio(x);
        printf("Il thread %d HA RILASCIATO la risorsa \t[%d]\n",*pi, x);

    }

    //printf("Thread%d partito: Hello World! Ho come identificatore %lu\n", *pi, pthread_self());
    /* pthread vuole tornare al padre un valore intero, ad es 1000+id */
    *ptr = x;
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

    myInit();

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

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, esegui, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    sleep(5);
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

