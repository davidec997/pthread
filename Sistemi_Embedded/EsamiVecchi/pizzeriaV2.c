
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
    sem_t clienti, *pizze,pizzaiolo;
    int b_entrata,b_turno,b_pizze;
    int posti, numero;
    int pizze_tot,pizze_rimanenti;
    int *ordini;
    int next;
}pizzeria;

void init_pizzeria (struct pizzeria_t *p){
    p->pizze = malloc(NUM_THREADS * sizeof (sem_t));

    sem_init(&p->pizzaiolo,0,0);
    pthread_mutex_init(&p->m,NULL);
    p->b_entrata = p->pizze_rimanenti = p->pizze_rimanenti = 0;
    p->posti = N;
    p->ordini = malloc((NUM_THREADS-1) * sizeof (int));
    for (int i = 0; i < NUM_THREADS-1; ++i){
        sem_init(&p->pizze[i],0,0);
        p->ordini[i] = 0;
    }
    p->next = -1;

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
    //controllo se c'e' posto
    if (p->posti > 0){
        p->posti --;
        printf("[CLIENTE %d]\t\tPosso entrare  --> %d posti liberi\n",*pi, p->posti);
        sem_post(&p->clienti);
        sem_post(&p->pizzaiolo); // ??
    } else{
        printf("[CLIENTE %d]\t\tNon posso entrare  --> %d posti liberi\n",*pi, p->posti);
     p->b_entrata  ++;
    }

    pthread_mutex_unlock(&p->m);
    sem_wait(&p->clienti);
}

void ordinaPizze (struct pizzeria_t * p, int * pi){

    pthread_mutex_lock(&p->m);
    int pizze = (rand()%6) + 1;
    p->ordini[*pi] = pizze;
    printf("[CLIENTE %d]\t\tHo ordinato e Sto aspettando le pizze\n",*pi);
    pthread_mutex_unlock(&p->m);

}

void ritiraPizze (struct  pizzeria_t * p , int * pi) {

    sem_wait(&p->pizze[*pi]);

    pthread_mutex_lock(&p->m);
    printf("[CLIENTE %d]\t\tHO PRESO LE MIE %d PIZZE\n",*pi,p->ordini[*pi]);
    // sveglio un cliente in coda
    if (p->b_entrata >0){
        p->b_entrata --;
        sem_post(&p->clienti);
    }
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

int nextOrder (struct pizzeria_t *p){
   int min = 0;
   int cliente = -1;
   int i;
    for (i=0; i<NUM_THREADS -1; i++)
        if (p->ordini[i]) {
            min = p->ordini[i];
            cliente = i;
            break;
        }

    while (i<NUM_THREADS - 1) {
        if (p->ordini[i] && min > p->ordini[i]) {
            min = p->ordini[i];
            cliente = i;
        }
        i++;
    }

    // se non ci sono ordini devo bloccarmi
    if (!min) {
        //pizzeria->c_p++; ??
        //sem_post(&pizzeria->m);
        pthread_mutex_unlock(&p->m);
       // sem_wait(&p->pizzaiolo); // passaggio del testimone
    }
    else
        p->next = cliente;
}

int prossima_pizza(struct pizzeria_t *p) {

    sem_wait(&p->pizzaiolo);
    pthread_mutex_lock(&p->m);
    // devo determinare il prossimo ordine da soddisfare
    //la politica e' che eseguo sempre il prossimo ordine con n di pizze minore
    int ord = 0;
    if (p->next == -1) ord = nextOrder(&pizzeria);
    printf("[PIZZAIOLO]\t\t\tORDINE DA EVADERE  %d --> pizze rimanenti : %d\n",ord,p->ordini[ord]);
    // faccio la pizza
    pthread_mutex_unlock(&p->m);

    return ord;
}

void consegna_pizza(struct pizzeria_t *p, int ord){

    pthread_mutex_lock(&p->m);
    p->ordini[ord] --;
    if (p->ordini[ord] == 0){
        printf("[PIZZAIOLO]\t\tHO FINITO L'ORDINE %d\n",ord);
        sem_post(&p->pizze[ord]);
        p->posti ++;
        p->next = -1;
    }

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
        printf("[PIZZAIOLO]\t\tciaa\n");
        ord = prossima_pizza(&pizzeria);
        //cuoci pizzza
        sleep(1);
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

    init_pizzeria(&pizzeria);
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



