
//DEADLOCK DOPO 10 ITER
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 4     //sedie
#define B 3
#define ITER 5
#define SHAVING_IT 2000000
#define PAY_ITER 100000
typedef enum {false, true} Boolean;

int sedie_libere;
pthread_mutex_t m;
pthread_cond_t cust,barb;
Boolean barbieri [B];
Boolean cassiere_occupato;
int clienti_in_attesa_fuori;

void myInit(void)
{
    pthread_mutex_init(&m,NULL);
    pthread_cond_init(&cust,NULL);
    pthread_cond_init(&barb,NULL);
    sedie_libere = N;
    for (int i =0;i<B;i++) barbieri[i]=false;
    cassiere_occupato = false;
    clienti_in_attesa_fuori = 0;

}

void *customerRoutine(int id) {
    while (1) {
        pthread_mutex_lock(&m);
        if (sedie_libere <= 0) {
            //vado via
            printf("Cliente %lu aspetta fuori perche' le sedie sono tutte occupate: %d libere\n",id, sedie_libere);
            pthread_mutex_unlock(&m);
            sleep(3);
            continue;
        } else {
            //pthread_mutex_lock(&m);
            sedie_libere--;
            printf("Sedie Libere  %d\n", sedie_libere);
            pthread_cond_signal(&cust);
            pthread_mutex_unlock(&m);
            pthread_cond_wait(&barb, &m);
            printf("Sono il cliente %lu e il barbiere mi sta servendo..\n", pthread_self());
            pthread_mutex_unlock(&m);
            //sleep(1);
        }
    }
}


void *barberRoutine(int id) {
    while(1){
        pthread_cond_wait(&barb,&m); // cust
        //pthread_mutex_lock(&m);
        sedie_libere ++;
        printf("Sono il barbiere e sto servendo un cliente\n");
        pthread_mutex_unlock(&m);
        pthread_cond_signal(&barb);
        sleep(3);

    }
}


int main (int argc, char **argv)
{
    pthread_t thread;

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

    pthread_create(&thread,NULL,barberRoutine,0);
    pthread_create(&thread,NULL,customerRoutine,1);
    pthread_create(&thread,NULL,customerRoutine,2);
    pthread_create(&thread,NULL,customerRoutine,3);
    pthread_create(&thread,NULL,customerRoutine,4);
    pthread_create(&thread,NULL,customerRoutine,5);
    /* for (i=0; i < NUM_THREADS ; i++) {

         taskids[i] = i;
         //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
         if (i == 0) { // il primo thread  e' il barbiere... gli altri sono clienti
             if (pthread_create(&thread[i], NULL, barberRoutine, (void *) (&taskids[i])) != 0) {
                 sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",
                         taskids[i]);
                 perror(error);
                 exit(5);
             }
         } else {
             if (pthread_create(&thread[i], NULL, customerRoutine, (void *) (&taskids[i])) != 0) {
                 sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",
                         taskids[i]);
                 perror(error);
                 exit(5);
             }
             printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
         }
     }*/

    sleep(5);
    /*for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        *//* attendiamo la terminazione di tutti i thread generati *//*
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }*/

    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);

    exit(0);
}
