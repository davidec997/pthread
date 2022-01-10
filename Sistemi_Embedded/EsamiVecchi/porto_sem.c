//
// Created by davide on 29/12/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


#define N 10 // posti barche


typedef enum {false, true} Boolean;

struct porto_t {
    pthread_mutex_t m;
    sem_t entrata, uscita;
    int b_entrata,b_uscita;
    int posti;
    int in_imboccatura;
}porto;

void init_porto (struct porto_t *p){

    sem_init(&p->entrata,0,0);
    sem_init(&p->uscita,0,0);

    pthread_mutex_init(&p->m,NULL);
    p->b_entrata = p->b_uscita = 0;
    p->posti = N;
    p->in_imboccatura = 0;

}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void entrata_richiesta (struct  porto_t * p, int *pi) {
    //per entrare nel porto devo controllare che ci siano meno di N barche gia ormeggiate
    // e che al piu una barca sta attraversando l'imboccatura

    pthread_mutex_lock(&p->m);
    if (p->posti > 0 && p->in_imboccatura < 2 && p->b_uscita == 0){
        p->in_imboccatura ++;
        sem_post(&p->entrata);
    } else{
        p->b_entrata ++;
        printf("[BARCA %d]\tNON POSSO ENTRARE XK NON CI SONO POSTI OPPURE IMBLOCCATURA FULL: %d o %d\n",*pi,p->posti, p->in_imboccatura);

    }

    pthread_mutex_unlock(&p->m);
    sem_wait(&p->entrata);
}

void entrata_ok (struct porto_t * p, int * pi){

    pthread_mutex_lock(&p->m);
    p->in_imboccatura --;
    p->posti --;

    if (p->b_uscita > 0) {
        p->in_imboccatura ++;
        p->b_uscita --;
        sem_post(&p->uscita);
    } else if (p->b_entrata > 0 && p->posti > 0){
        p->b_entrata --;
        p->in_imboccatura ++;
        sem_post(&p->entrata);
    }

    pthread_mutex_unlock(&p->m);

}

void uscita_richiesta (struct  porto_t * p , int * pi) {

    pthread_mutex_lock(&p->m);
    if (p->in_imboccatura < 2){
        p->in_imboccatura ++;
        sem_post(&p->uscita);
    } else
        p->b_uscita ++;

    pthread_mutex_unlock(&p->m);
    sem_wait(&p->uscita);
}

void uscita_ok (struct porto_t * p, int *pi){

    pthread_mutex_lock(&p->m);
    p->in_imboccatura --;
    p->posti ++;

    if (p->b_uscita > 0) {
        p->in_imboccatura ++;
        p->b_uscita --;
        sem_post(&p->uscita);
    } else if (p->b_entrata > 0 && p->posti > 0){
        p->b_entrata --;
        p->in_imboccatura ++;
        sem_post(&p->entrata);
    }


    pthread_mutex_unlock(&p->m);
}


void *barca (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //senza ciclo
    while(1){
        sleep(rand()%20);
        pausetta();
        printf("[BARCA %d]\tSono arrivata all'imboccatura del porto...\n",*pi);
        entrata_richiesta(&porto,pi);
        printf("[BARCA %d]\tHo ottenuto l'autorizzazione ad entrare nel porto...\n",*pi);
        pausetta();
        entrata_ok(&porto,pi);
        printf("[BARCA %d]\tSono ormeggiata nel porto...\n",*pi);
        printf("[PORTO]\t\t POSTI DIPONIBILI : %d\n",porto.posti);
        sleep(5);
        uscita_richiesta(&porto,pi);
        printf("[BARCA %d]\tHo ottenuto l'autorizzazione ad uscire dal porto...\n",*pi);
        uscita_ok(&porto,pi);
        printf("[BARCA %d]\tSono fuori dal porto...\n",*pi);
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

    init_porto(&porto);
    srand(555);

    for (i=1; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, barca, (void *) (&taskids[i])) != 0)
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
        printf("Pthread %d-esimo restituisce %d \n", i, ris);
    }

    exit(0);
}
