//SEMBRA FUNZIONARE
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 4
#define ITER 5
#define DELAY 2000000
typedef enum {false, true} Boolean;

int sedie_libere;
sem_t m, barber, custumers;
Boolean sedia_barb_occ;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&barber, 0, 1);
    sem_init(&custumers, 0, 0);
    sedie_libere = N;

}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void servi(){
    sem_wait(&custumers); //barber
    sem_wait(&m);
    sedie_libere ++;
    sem_post(&barber); //cust
    sem_post(&m);
    printf(" Sono il Barbiere e Sto servendo un cliente...\n ");
    sleep(2);
}


void richiedi_servizio(int pi){
    sem_wait(&m);
    if(sedie_libere <=0) {
        printf("Il cliente %d ha trovato tutte le sedie occupate e se ne va...\n",pi);
        sem_post(&m);
        sleep(2);
    } else {
        sedie_libere--;
        printf("SEDIE LIBERE %d\n", sedie_libere);
        sem_post(&custumers); //barb
        sem_post(&m);
        sem_wait(&barber); // cu
        printf("SONO IL CLIENTE %d IL BARBIERE MI STA SERVENDO...\n", pi);
        sleep(1);
    }
}

void *customerRoutine(int id) {
    //int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    printf("Sono il thread cliente %d\n",id);
    for (;;) {
        richiedi_servizio(id);
    }

    //printf("Thread%d partito: Hello World! Ho come identificatore %lu\n", *pi, pthread_self());
    /* pthread vuole tornare al padre un valore intero, ad es 1000+id */
    *ptr = NULL;
    pthread_exit((void *) ptr);
}



void *barberRoutine(int id) {
    //int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    int clineti_serviti =1;
    printf("Sono il thread barbiere %d\n",id);
    for (;;){
        //sleep(1);
        servi();
        printf(" Sono il barbiere e Ho servito  %d clienti ...\n",clineti_serviti);
        clineti_serviti ++;
    }

    //printf("Thread%d partito: Hello World! Ho come identificatore %lu\n", *pi, pthread_self());
    /* pthread vuole tornare al padre un valore intero, ad es 1000+id */
    *ptr = NULL;
    pthread_exit((void *) ptr);
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

//
// Created by dada on 02/11/21.
//

