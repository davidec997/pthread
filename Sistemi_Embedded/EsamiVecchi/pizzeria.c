//
// Created by dada on 31/12/21.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


#define N 10 // posti in pizzeria


typedef enum {false, true} Boolean;

struct pizzeria_t {
    pthread_mutex_t m;
    pthread_cond_t entrata,turno,pizze;
    int b_entrata,b_turno,b_pizze;
    int posti;
    int pizze_tot,pizze_rimanenti;
    int ordini [N];
}pizzeria;

void init_porto (struct pizzeria_t *p){
    pthread_cond_init(&p->entrata,NULL);
    pthread_cond_init(&p->turno,NULL);
    pthread_cond_init(&p->pizze,NULL);

    pthread_mutex_init(&p->m,NULL);
    p->b_entrata = p->pizze_rimanenti = p->pizze_rimanenti = 0;
    p->posti = N;
    for (int i = 0; i < N; ++i) p->ordini[i] = 0;

}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void entrata_richiesta (struct  pizzeria_t * p, int *pi) {
    //per entrare nel porto devo controllare che ci siano meno di N barche gia ormeggiate
    // e che al piu una barca sta attraversando l'imboccatura

    pthread_mutex_lock(&p->m);
    while (p->posti <= 0){
        printf("[BARCA %d]\t\t\tMi blocco perche non ci sono posti in pizzeria: POSTI LIBERI: %d\n",*pi,p->posti);
        p->b_entrata ++;
        pthread_cond_wait(&p->entrata,&p->m);
        p->b_entrata --;
    }
    printf("[BARCA %d]\t\t\tPosso entrare in pizzeria\tPOSTI LIBERI: %d\n",*pi,p->posti);
    p->posti --;


    pthread_mutex_unlock(&p->m);
}

void entrata_ok (struct pizzeria_t * p, int * pi){

    pthread_mutex_lock(&p->m);

    p->pizze_tot = p->pizze_rimanenti = (rand() % 6) + 1;

    pthread_mutex_unlock(&p->m);

}

void uscita_richiesta (struct  pizzeria_t * p , int * pi) {

    pthread_mutex_lock(&p->m);



    pthread_mutex_unlock(&p->m);
}



void *cliente (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //senza ciclo
    pausetta();
    printf("[CLIENTE %d]\tSono arrivata all'imboccatura del porto...\n",*pi);
    entrata_richiesta(&pizzeria,pi);
    printf("[CLIENTE %d]\tHo ottenuto l'autorizzazione ad entrare nel porto...\n",*pi);
    entrata_ok(&pizzeria,pi);
    printf("[CLIENTE %d]\tSono ormeggiata nel porto...\n",*pi);
    sleep(2);
    uscita_richiesta(&pizzeria,pi);
    printf("[CLIENTE %d]\tSono fuori dal porto...\n",*pi);

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void prossima_pizza(struct pizzeria_t *p) {


}

void consegna_pizza(struct pizzeria_t *p){

}

void *pizzaiolo (void * arg){
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }


    prossima_pizza(&pizzeria);
    //cuoci pizzza
    consegna_pizza();

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

    init_porto(&pizzeria);
    srand(555);

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        if (i == NUM_THREADS -1) pthread_create(&thread[i], NULL, pizzaiolo, (void *) (&taskids[i]));
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, cliente, (void *) (&taskids[i])) != 0)
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

