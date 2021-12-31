//
// Created by davide on 27/12/21.
//
//DA AGGIUSTARE
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 4
#define NTIMES 5
#define BARBIERI 3

typedef enum {false, true} Boolean;

int sedie_libere;
sem_t m, *barber, custumers,cassiere,stop;
int clienti_in_attesa,cli_blocc_barb;
Boolean barbieri_occupati[3];


void myInit(void)
{
    barber = malloc(BARBIERI * sizeof (sem_t));
    for (int i = 0; i < BARBIERI; i++) {
        sem_init(&barber[i],0,0);
        barbieri_occupati[i] = false;
    }
    sem_init(&m,0,1);
    sem_init(&cassiere,0,1);
    sem_init(&stop,0,0);


    sem_init(&custumers, 0, 0);
    sedie_libere = N;
    clienti_in_attesa = 0;
    cli_blocc_barb = 0;

}


void servi( int * pi){

    printf("[BARBIERE %d] Ciao aspetto un cliente\n",*pi);
    sem_wait(&custumers);
    sem_wait(&m);
    barbieri_occupati[*pi] = true;
    sedie_libere ++;
    printf("\tSono il Barbiere %d e Sto servendo un cliente...\n ",*pi);
    sem_post(&m);
    sleep(5);

    sem_wait(&m);
    barbieri_occupati[*pi] = false;
    sem_post(&m);
    sem_post(&barber[*pi]);

}


int checkBarber(){
    int index = -1;
    if (barbieri_occupati[0] == false)
        index = 0;
    else if (barbieri_occupati[1] == false)
        index = 1;
    else if (barbieri_occupati[2] == false)
        index = 2;
    return index;
}


void richiedi_servizio(int pi, int * taglio){
    int * ptr;
    ptr = (int *) malloc(sizeof(int));
    sem_wait(&m);
    int i;
    //se mi sono gia tagliato i capelli me ne vado def
    /*if (*taglio > 0){
        *ptr = *taglio;
        sem_post(&m);
        pthread_exit((void *) ptr);
    }*/

    if(sedie_libere <=0 ) {
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
    }
}

void paga (int * pi){
    sem_wait(&cassiere);
    printf("[CASSIERE] Il cliente %d sta pagando\n",*pi);
    if (checkBarber() != -1 && cli_blocc_barb > 0){
        sem_wait(&m);
        sem_post(&stop);
        cli_blocc_barb --;
        printf("Cassiere ne ha svegliato 1\n");
        sem_post(&m);
    }
    sleep(1);
    sem_post(&cassiere);
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
        paga(pi);
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
        servi(pi);
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

    for (i=1; i < NUM_THREADS ; i++)
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
        printf("Pthread %d-esimo restituisce %d  -->numero di volte che si e' tagliato i capelli\n", i, ris);
    }

    //pthread_mutex_destroy(&m);
    exit(0);
}




