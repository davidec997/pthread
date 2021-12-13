//FUNZIONA.. TUTTI I CLIENTI RIESCONO A TAGLIARSI I CAPELLI PROVANDO AL MASSIMO 5 VOLTE
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 4
#define NTIMES 5

//typedef enum {false, true} Boolean;

int sedie_libere;
sem_t m, barber, custumers;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&barber, 0, 0);
    sem_init(&custumers, 0, 0);
    sedie_libere = N;

}


void servi(){
    sem_wait(&custumers);
    sem_wait(&m);
    sedie_libere ++;
    printf("\tSono il Barbiere e Sto servendo un cliente...\n ");
    sleep(1);
    sem_post(&m);
    sem_post(&barber);

}


void richiedi_servizio(int pi, int * taglio){
    int * ptr;
    ptr = (int *) malloc(sizeof(int));
    sem_wait(&m);
    //se mi sono gia tagliato i capelli me ne vado def
    if (*taglio > 0){
        *ptr = *taglio;
        sem_post(&m);
        pthread_exit((void *) ptr);
    }

    if(sedie_libere <=0) {
        printf("Il cliente %d ha trovato tutte le sedie occupate e se ne va...\n",pi);
        sem_post(&m);
        sleep(3);
    } else {
        sedie_libere--;
        printf("SEDIE LIBERE %d\n", sedie_libere);
        sem_post(&custumers); //sveglio il barb
        *taglio += 1;
        printf("SONO IL CLIENTE %d IL BARBIERE MI STA SERVENDO...\n", pi);
        sem_post(&m);
        sem_wait(&barber);
        sleep(1);
    }
}

void *customerRoutine(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int *taglio = 0;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    printf("Sono il thread cliente %d\n",*pi);

    for (int f =0; f< NTIMES; f++) {
        richiedi_servizio(*pi,&taglio);
    }

    *ptr = taglio;
    pthread_exit((void *) ptr);

}


void *barberRoutine(void * id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    int clineti_serviti =1;
    printf("Sono il thread barbiere %d\n",*pi);
    for (;;){
        //sleep(1);
        servi();
        printf("\tSono il barbiere e Ho servito  %d clienti ...\n",clineti_serviti);
        clineti_serviti ++;
    }

    *ptr = *pi;
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

    //genero il trhead 0 barbiere
    pthread_create(&thread[0], NULL, barberRoutine, (void *) (&taskids[0]));

    for (i=1; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
       // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, customerRoutine, (void *) (&taskids[i])) != 0)
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
        /* attendiamo la terminazione di tutti i thread customers */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d  -->numero di volte che si e' tagliato i capelli\n", i, ris);
    }

    //pthread_mutex_destroy(&m);
    exit(0);
}


