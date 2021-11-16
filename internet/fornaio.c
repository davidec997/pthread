// DA VEDERE XK NON SI SVEGLIANO QUANDO IL FORNAIO FA SIGNAL
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 10
#define A 10
#define ITER 50
typedef enum {false, true} Boolean;

int bigliettoGlob,bigliettoDisplay,clienti_attesa,pagnotte;
pthread_cond_t sveglia_cliente_succ,fornaio;
pthread_mutex_t m_distributore_biglietti,m_turno;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void myInit(void)
{
    pthread_mutex_init(&m_distributore_biglietti,NULL);
    pthread_mutex_init(&m_turno,NULL);
    pthread_cond_init(&sveglia_cliente_succ,NULL);
    pthread_cond_init(&fornaio,NULL);

    bigliettoDisplay = bigliettoGlob = clienti_attesa = 0;
    pagnotte = N;
}

void sfornaPgnotte(){
    pthread_mutex_lock(&m_turno);
    sleep(1);
    pagnotte = N;
    pthread_mutex_unlock(&m_turno);
    pthread_cond_broadcast(&fornaio);
    pthread_cond_broadcast(&sveglia_cliente_succ);

}

int prendiBiglietto(){
    int biglietto;
    pthread_mutex_lock(&m_distributore_biglietti);
    biglietto= bigliettoGlob;
    bigliettoGlob ++;
    printf("Preso il biglietto %d\n",biglietto);
    pthread_mutex_unlock(&m_distributore_biglietti);
    return biglietto;

}

void aspetta_turno(int biglietto){
    pthread_mutex_lock(&m_turno);
    while(bigliettoDisplay != biglietto){
        clienti_attesa ++;
        printf("Un cliente si e' messo in attesa biglietto %d\n",biglietto);
        pthread_cond_wait(&sveglia_cliente_succ,&m_turno);
        clienti_attesa --;
    }
    while (pagnotte <= 0){
        sfornaPgnotte();
        pthread_cond_wait(&fornaio,&m_turno);
    }
    printf("Il cliente con bilietto %d si sta servendo... Pagnotte rimanenti %d \n\n",biglietto,pagnotte);
    sleep(1);
    bigliettoDisplay ++;
    pagnotte --;
    pthread_mutex_unlock(&m_turno);
    pthread_cond_broadcast(&sveglia_cliente_succ);
}


void *esegui(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int ret = 0;
    int biglietto;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    // printf("sto per iniziare thread %d\n",(int*)pi);
    for (int t = 0; t < ITER; t++){
        biglietto = prendiBiglietto();
        aspetta_turno(biglietto);
    }

    /* pthread vuole tornare al padre un valore intero, ad es 1000+id */
    *ptr = NULL;
    pthread_exit((void *) ret);
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


