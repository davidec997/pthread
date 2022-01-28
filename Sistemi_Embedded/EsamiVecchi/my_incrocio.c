#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define LAMPEGGIANTE -1
#define ROSSO 0
#define GIALLO 2
#define VERDE 1
#define N 10 // dimensione coda; max macchine ammesse nella coda
#define MAX_DELAY 30
#define N_STAMPE 120
#define SEM_TIMER 3


typedef enum {false, true} Boolean;
char *colori [3] = {"ROSSO", "GIALLO", "VERDE"};

struct incrocio_t {
    sem_t s[4],m;
    int stato;
    int coda[4];
    int occupato[4];

}incrocio;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}
void init_incrocio ( struct  incrocio_t *in){
    sem_init(&in->m,0,1);

    for (int i = 0; i < 4; i++) {
        sem_init(&in->s[i],0,0);
        in->coda[i] = 0;
        in->occupato[i] = 0;
    }

    in->stato = LAMPEGGIANTE;
}

void sveglia_bloccati (struct  incrocio_t * in){
    sem_wait(&in->m);
    for (int i =0 ; i< 4; i++)
        if (in->coda[i] > 0)
            sem_post(&in->s[i]);
    sem_post(&in->m);
}

void rosso (struct incrocio_t * in ){
    sem_wait(&in->m);
    in->stato = ROSSO;
    printf("\t\t\tROSSO\n");
    sem_post(&in->m);
}

void verde (struct incrocio_t * in ){
    sem_wait(&in->m);
    in->stato = VERDE;
    printf("\t\t\tVERDE\n");
    sem_post(&in->m);
    sveglia_bloccati(in);
}

void giallo (struct incrocio_t * in ){
    sem_wait(&in->m);
    in->stato = GIALLO;
    printf("\t\t\tGIALLO\n");

    sem_post(&in->m);
}

void *timer (){
    while (1) {
        pausetta();
        rosso(&incrocio);
        sleep(SEM_TIMER);
        verde(&incrocio);
        sleep(SEM_TIMER - 1);
        giallo(&incrocio);
        sleep(1);
    }
}

void controlla_attraversamento (struct incrocio_t *in, int inc){
    sem_wait(&in->m);

    in->coda[inc] ++;
    if ((in->stato == VERDE || in->stato == LAMPEGGIANTE) && in->coda[inc] == 1 && in->occupato[inc] == 0){
        // posso passare
        sem_post(&in->s[inc]);
        in->occupato[inc] ++;
    }

    sem_post(&in->m);
    sem_wait(&in->s[inc]);
}

void termina_attraversamento ( struct incrocio_t *in , int inc){
    sem_wait(&in->m);
    in->occupato[inc] --;
    in->coda[inc] --;

    if (in->stato == VERDE && in->coda[inc] > 0){
        sem_post(&in->s[inc]);
        in->occupato[inc] ++;
    }

    sem_post(&in->m);

}

void *automobile (void * id){
    int *pi = (int *)id;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    int n_attraversamenti=0;
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    int inc = rand()%4;
    printf("[AUTO %d]\tSONO ARRIVATA ALL'INCROCIO %d\n",*pi,inc);
    controlla_attraversamento(&incrocio, inc);
    printf("[AUTO %d]\tSTO ATTRAVERSANDO L'INCROCIO %d.....\n",*pi,inc);
    sleep(1);
    termina_attraversamento(&incrocio, inc);
    printf("[AUTO %d]\tLIBERO L'INCROCIO %d\n",*pi,inc);


    *ptr = n_attraversamenti;
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

    init_incrocio(&incrocio);

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


    srand(555);

    taskids[0] = 0;
    pthread_create(&thread[i], NULL, timer, (void *) (&taskids[0]));
    sleep(2);

    for (i=1; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, automobile, (void *) (&taskids[i])) != 0)
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

    exit(0);
}
