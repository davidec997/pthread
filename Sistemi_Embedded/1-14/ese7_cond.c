//
// Created by davide on 27/12/21.
//
//DA AGGIUSTARE
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 4 // sedie libere
#define NTIMES 5
#define BARBIERI 3
#define CLIENTI 20

typedef enum {false, true} Boolean;

struct barbiere_t {
    Boolean occupato;
    int clienti_serviti;
    pthread_cond_t barb;
}barbiere;

struct negozio_t {
    int sedie_libere, clienti_fuori,attesa_barb ;
    pthread_cond_t custumers, poltrone,stop;
    struct barbiere_t barbiere[BARBIERI];
    pthread_mutex_t m, cassiere;
}negozio;


void myInit(struct negozio_t *n)
{
    for (int i = 0; i < BARBIERI; i++) {
        pthread_cond_init(&n->barbiere[i].barb,NULL);
        n->barbiere[i].occupato = false;
        n->barbiere[i].clienti_serviti = 0;
    }
    pthread_mutex_init(&n->m,NULL);
    pthread_cond_init(&n->custumers,NULL);
    pthread_mutex_init(&n->cassiere,NULL);
    pthread_cond_init(&n->stop,NULL);



    n->sedie_libere = N;
    n->clienti_fuori = 0;
    n->attesa_barb = 0;
}


void servi( struct negozio_t *n, int *pi){

    pthread_mutex_lock(&n->m);
    while (n->attesa_barb == 0){
        printf("[BARBIERE %d]\t SERVO CLIENTE\n",*pi);
         pthread_cond_wait(&n->barbiere[*pi].barb,&n->m);
    }

    n->barbiere[*pi].occupato = true;
    printf("[BARBIERE %d   %d]\t SERVO CLIENTE\n",*pi);
    //pthread_mutex_unlock(&n->m);

    sleep(1);
   // pthread_mutex_lock(&m);
    n->sedie_libere ++;
    pthread_cond_signal(&n->custumers);
    n->barbiere[*pi].occupato = false;

    pthread_mutex_unlock(&n->m);
    //pthread_cond_signal(&stop);

}


int checkBarber(struct  negozio_t *n){
    int index = -1;
    if (n->barbiere[0].occupato == false)
        index = 0;
    else if (n->barbiere[1].occupato == false)
        index = 1;
    else if (n->barbiere[2].occupato == false)
        index = 2;
    return index;
}


void richiedi_servizio(struct negozio_t *n, int pi){
    int * ptr;
    ptr = (int *) malloc(sizeof(int));
    int i;
    //controllo se ci sono poltrone libere
    pthread_mutex_lock(&n->m);
    while (n->sedie_libere <= 0){
        n->clienti_fuori ++;
        pthread_cond_wait(&n->poltrone,&n->m);
        n->clienti_fuori ++;
    }

    printf("[SITUA]: B1 [%d] B2 [%d] B3 [%d]\n",n->barbiere[0].occupato,n->barbiere[1].occupato,n->barbiere[2].occupato);
    while (checkBarber(n) == -1){
        //n-> attesa_barb++;
        pthread_cond_wait(&n->stop,&n->m);
        //cli_blocc_barb --;
    }

    //pthread_cond_signal(&custumers);
    i = checkBarber(n);
    if (i == -1) printf("ERROR\n");

    printf("[CLIENTE %d] SI FA TAGLIARE I CAPELLI DAL BARBIERE %d\n",pi,i);
    pthread_cond_signal(&n->custumers);
    pthread_cond_wait(&n->barbiere[i].barb,&n->m);


    printf("[CLIENTE %d] MI HA TAGLIATO I CAPELLI IL BARBIERE %d\n",pi,i);
    pthread_mutex_unlock(&n->m);
    /*if(sedie_libere <=0 ) {
        printf("Il cliente %d ha trovato tutte le sedie occupate e se ne va...\n",pi);
        sem_post(&m);
        sleep(3);
    } else {
        i = checkBarber();
        if (i== -1){
            printf("TUTTI I BARBIERI SONO OCCUPATI... Mi Blocco\n");
            cli_blocc_barb ++;
            sem_post(&m);
            sem_wait(&stop);
            printf("Il cliente %d di e svegliato \n",pi);
            sem_wait(&m);
        }
        sedie_libere--;
        printf("SEDIE LIBERE %d\n", sedie_libere);
        sem_post(&custumers); //sveglio il barb
        *taglio += 1;
        printf("SONO IL CLIENTE %d IL BARBIERE MI STA SERVENDO...\n", pi);
        sem_post(&m);
        sem_wait(&barber[i]);
        printf("SONO IL CLIENTE %d IL BARBIERE MI HA SERVITO...\n", pi);

        sleep(1);
    }*/
}

void paga (struct negozio_t *n, int * pi){
    pthread_mutex_lock(&n->cassiere);

    printf("[CASSIERE] UN CLIENTE MI STA PAGANDO \n");
    sleep(1);
    pthread_mutex_unlock(&n->cassiere);

    pthread_mutex_lock(&n->m);
    if (n->clienti_fuori>0 && n->sedie_libere > 0){
        n->clienti_fuori --;
        pthread_cond_signal(&n->poltrone);
    }
    pthread_mutex_unlock(&n->m);

}

void *customerRoutine(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int *taglio;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    printf("Sono il thread cliente %d\n",*pi);

    //for (;;) {
        sleep(1);
        richiedi_servizio(&negozio,*pi);
        paga(&negozio, pi);
    //}

    *ptr = *pi;
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
        servi(&negozio, pi);
        printf("\tSono il barbiere %d e Ho servito  %d clienti ...\n",*pi,clineti_serviti);
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

    myInit(&negozio);

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

    for (i=0; i < NUM_THREADS ; i++)
    {
        taskids[i] = i; // 3 barbieri
        if ( i == 1 || i == 0 || i == 2) pthread_create(&thread[0], NULL, barberRoutine, (void *) (&taskids[i]));

        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, customerRoutine, (void *) (&taskids[i])) != 0)
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
        /* attendiamo la terminazione di tutti i thread customers */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d \n", i, ris);
    }

    //pthread_mutex_destroy(&m);
    exit(0);
}




