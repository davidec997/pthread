//FUNZIONA
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define M 5
#define K 6
#define NTIMES 6

typedef enum {false, true} Boolean;
Boolean addettoAttivo;
pthread_cond_t vuoto,pieno;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int residuo [M];

void myInit(){
    for (int i = 0; i < M; i++) residuo[i] = K;
    pthread_cond_init(&vuoto,NULL);
    pthread_cond_init(&pieno,NULL);
    addettoAttivo = false;

}


void svuotaVassoio(){
    printf("Sono L ADDETTO  e STO DORMENDO\n");

    pthread_mutex_lock(&m);
    while(!addettoAttivo)
        pthread_cond_wait(&pieno,&m);
        //pthread_mutex_lock(&m);  SE PRENDO IL MUTEX QUI VA IN DEADLOCK !!!!!!!



    printf("Sono L ADDETTO  e SONO STATO SVEGLIATO\n");
    for (int i = 0; i < M; i++) residuo [i] = K;
    sleep(1);
    addettoAttivo = false;
    printf("Sono L ADDETTO  e HO SVUOTATO I VASSOI\n");

    pthread_mutex_unlock(&m);
    pthread_cond_broadcast(&vuoto);

}

void *eseguiAddetto(void * id){
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("Ciao sono il thread ADDETTO  %d \n",*pi);

    while (1){
        svuotaVassoio();
    }

    *ptr = 1000+*pi;
    pthread_exit((void *) ptr);
}

int controllaSpazio(int spazio){
    int i,index;
    index = -1;
    //pthread_mutex_lock(&m);
    for (i = 0; i < M; i++) {
        if (residuo[i] >= spazio){
            index = i;
            break;
        }
    }
    //pthread_mutex_unlock(&m);
    return index;
}

void chiamaAddetto(int id){
    printf("Thread %d sta svegliando l addetto \n",id);
    addettoAttivo = true;
    pthread_cond_signal(&pieno);

}

void *eseguiCliente(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int r,index;
    int posti_occupati = 0;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (int t = 0; t < NTIMES; ++t) {

        r = rand() % 2 + 1;
        pthread_mutex_lock(&m);

        printf("\nCiao sono il thread CLIENTE %d e devo lasciare un vassoio che richiede %d  posti\n",*pi,r);
        index = controllaSpazio(r);

        while( (controllaSpazio(r) == -1)){
            if (!addettoAttivo) chiamaAddetto(*pi);
            pthread_cond_wait(&vuoto,&m);
        }

        sleep(1);
        residuo[index] -= r;
        posti_occupati += r;
        printf("Thread %d ha lasciato il vassoio in residuo [%d] occupando %d posti e se ne va\n",*pi,index,r);
        printf("\t SITUAZIONE ATTUALE:\n");
        for (int t = 0; t < M; t++) {
            printf(" Nel vasoio %d\t[%d] liberi\n",t,residuo[t]);
        }
        pthread_mutex_unlock(&m);

    }

    *ptr = posti_occupati;
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

    pthread_create(&thread[0], NULL, eseguiAddetto, (void *) (&taskids[0]));
    // addetto
    sleep(3);

    for (i=1; i < NUM_THREADS; i++)
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

    for (i=1; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d <- tot posti occupati\n", i, ris);
    }

    pthread_mutex_destroy(&m);
    exit(0);
}




