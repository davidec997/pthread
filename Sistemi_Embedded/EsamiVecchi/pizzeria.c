//
// Created by dada on 31/12/21.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


#define N 6 // posti in pizzeria


typedef enum {false, true} Boolean;
int NUM_THREADS;


struct pizzeria_t {
    pthread_mutex_t m;
    pthread_cond_t entrata,turno,*pizze;
    int b_entrata,b_turno,b_pizze;
    int posti, numero;
    int pizze_tot,pizze_rimanenti;
    int *ordini;
}pizzeria;

void init_porto (struct pizzeria_t *p){
    pthread_cond_init(&p->entrata,NULL);
    pthread_cond_init(&p->turno,NULL);

    p->pizze = malloc((NUM_THREADS-1) * sizeof (pthread_cond_t));
    for (int i = 0; i < NUM_THREADS-1; i++) pthread_cond_init(&p->pizze[i],NULL);

    pthread_mutex_init(&p->m,NULL);
    p->b_entrata = p->pizze_rimanenti = p->pizze_rimanenti = 0;
    p->posti = N;
    p->ordini = malloc((NUM_THREADS-1) * sizeof (int));
    for (int i = 0; i < NUM_THREADS-1; ++i) p->ordini[i] = 0;
    p->numero = 0;

}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void entrata_richiesta (struct  pizzeria_t * p, int *pi) {

    pthread_mutex_lock(&p->m);
    while (p->posti <= 0){
        printf("[CLIENTE %d]\t\t\tMi blocco perche non ci sono posti in pizzeria: POSTI LIBERI: %d\n",*pi,p->posti);
        p->b_entrata ++;
        pthread_cond_wait(&p->entrata,&p->m);
        p->b_entrata --;
    }
    p->posti --;

    printf("[CLIENTE %d]\t\t\tPosso entrare in pizzeria\tPOSTI LIBERI: %d\n",*pi,p->posti);

    pthread_mutex_unlock(&p->m);
}

void ordinaPizze (struct pizzeria_t * p, int * pi){

    pthread_mutex_lock(&p->m);

    pausetta();
    int pizzetot = (rand() % 6) + 1;
    p->ordini[*pi] = pizzetot;

    printf("[CLIENTE %d]\t\t\tHO ORDINATO %d PIZZE\n",*pi,pizzetot);

    pthread_mutex_unlock(&p->m);

}

void ritiraPizze (struct  pizzeria_t * p , int * pi) {

    pthread_mutex_lock(&p->m);

    /*while (p->ordini[*pi] >= 0){
        printf("[CLIENTE %d]\t\t\tSTO ASPETTANDO ALTRE PIZZE: NE MANCANO %d \n",*pi,p->ordini[*pi]);
    }*/
    printf("[CLIENTE %d]\t\t\tOOOOOOOOOOOOOO: %d\n",*pi,p->ordini[*pi]);

    pthread_cond_wait(&p->pizze[*pi],&p->m);
    printf("[CLIENTE %d]\t\t\tHO PRESO TUTTE LE MIE PIZZE: %d\n",*pi,p->ordini[*pi]);
    p->posti ++;

    // sveglio un cliente fuori dalla porta
    if (p->b_entrata > 0) pthread_cond_signal(&p->entrata);

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
    printf("[CLIENTE %d]\tSono alla pizzeria...\n",*pi);
    entrata_richiesta(&pizzeria,pi);
    //printf("[CLIENTE %d]\tSono entrato in pizzeria...Scelgo le pizze\n",*pi);
    ordinaPizze(&pizzeria,pi);
    //printf("[CLIENTE %d]\tHo ordinato le pizze...\n",*pi);
    //sleep(2);
    ritiraPizze(&pizzeria,pi);
    printf("[CLIENTE %d]\tSto ANDANDO A MANGIARE LE PIZZE...\n",*pi);

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int prossima_pizza(struct pizzeria_t *p) {

    pthread_mutex_lock(&p->m);
    // devo determinare il prossimo ordine da soddisfare
    //la politica e' che eseguo sempre il prossimo ordine con n di pizze minore

    int ord = 0;
    int c = 0;
    int min_ele = p->ordini[0] ;

    for ( c = 1 ; c < NUM_THREADS -1 ; c++ ){
        printf("\t\t\t%d\t",p->ordini[c]);

        if ( p->ordini[c] < min_ele && p->ordini[c] > 0 ){
            min_ele = p->ordini[c];
            ord = c;
        }
    }

    pthread_mutex_unlock(&p->m);
    sleep(1);
    printf("[PIZZAIOLO]\t\tINIZIO A PREPARARE PIZZE PER IL CLIENTE %d\n",ord);

    pthread_mutex_lock(&p->m);

    while (p->ordini[ord] > 0) {
        //prepara pizza
        sleep(1);
        p->ordini[ord] --;
        printf("[PIZZAIOLO]\t\tPIZZA PER IL CLIENTE %d PRONTA. NE RESTANO %d\n",ord,p->ordini[ord]);
    }

    // finito l'ordine
    //sleep(1);
    p->ordini[ord] = 0;
    printf("\t\t\t%d ORD e %d pizze di ORD\n",ord, p->ordini[ord]);
    pthread_mutex_unlock(&p->m);
    return ord;

}

void consegna_pizza(struct pizzeria_t *p, int ord){

    pthread_mutex_lock(&p->m);
    // sveglio il cliente
    printf("[PIZZAIOLO]\t\t CLIENTE %d PRONTO X andarsene. \n",p->ordini[ord]);

    pthread_cond_signal(&p->pizze[ord]);

    pthread_mutex_unlock(&p->m);

}

void *pizzaiolo (void * arg){
    int *pi = (int *)arg;
    int *ptr;
    int ord;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    sleep(2);

    while (1){
        ord = prossima_pizza(&pizzeria);
        //cuoci pizzza
        consegna_pizza(&pizzeria,ord);
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

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d \n", i, ris);
    }

    exit(0);
}

