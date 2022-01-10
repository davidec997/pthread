
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define TORTE_MAX 6
#define TORTE_MIN 3

typedef enum {false, true} Boolean;

struct pasticceria_t {
    sem_t  pasticcere, torte_pronte, servimi, servito, m;
    int torte_disp;
    Boolean pasticcere_blocc;

}pasticceria;

void init_mr (struct pasticceria_t * p){
    sem_init(&p->m,0,1);
    sem_init(&p->pasticcere,0,0);
    sem_init(&p->servimi,0,0);
    sem_init(&p->servito,0,0);
    sem_init(&p->torte_pronte,0,0);

    p->torte_disp = 0;
    p->pasticcere_blocc = false;

}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void acquista_torta (struct pasticceria_t *p, int *pi){
    sem_post(&p->servimi);
    printf("[CLIENTE %d] IN ATTESA DI UNA TORTA\n",*pi);
    sem_wait(&p->servito);
    printf("[CLIENTE %d] VADO A MANGIARE LA MIA TORTA\n",*pi);

}

void *cliente (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);}

    for(int i =0; i< 10;i++) {
        sleep((rand()%10 + 8));
        acquista_torta(&pasticceria,pi);
    }
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void inizio_preparazione(struct pasticceria_t *p){
    sem_wait(&p->m);
    if (p->torte_disp < TORTE_MAX){
        sem_post(&p->pasticcere);
    } else {
        p->pasticcere_blocc = true;
        printf("[PASTICCERE] MI BLOCCO XK CI SONO GIA %d TORTE\n",p->torte_disp);

    }

    sem_post(&p->m);
    sem_wait(&p->pasticcere);
    printf("[PASTICCERE] PREPARO TORTA\n");

    pausetta();
}

void fine_preparazione(struct pasticceria_t *p){
    sem_wait(&p->m);

    p->torte_disp ++;
    printf("[PASTICCERE]  TORTA  FATTA... ORA CE NE SONO %d\n",p->torte_disp);

    sem_post(&p->torte_pronte);
    sem_post(&p->m);
}


void prepara_torta(struct pasticceria_t *p){
    sleep(1);
}
void *pasticcere (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (;;){
        inizio_preparazione(&pasticceria);
        pausetta();
        prepara_torta(&pasticceria);
        pausetta();
        fine_preparazione(&pasticceria);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void commesso_prende_torta(struct pasticceria_t *p){
    sem_wait(&p->torte_pronte);
    printf("[COMMESSO] PRENDO UNA TORTA E MENTRE LA INCARTO ASPETTO UN CLIENTE\n");
    sem_wait(&p->servimi);
    printf("[COMMESSO] E' ARRIVATO UN CLIENTE\n");

}

void commesso_vende_torta(struct pasticceria_t *p){
    sem_wait(&p->m);
    p->torte_disp --;
    printf("[COMMESSO]\t\tSERVITO CLIENTE... TORTE DISP: %d\n",p->torte_disp);
    sem_post(&p->servito);
    if (p->torte_disp <= 0){
        sem_post(&p->pasticcere);
        printf("[COMMESSO]\t\t\tSVEGLIO IL PASTICCERE\n");
    }
    sem_post(&p->m);
}


void *commesso (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (;;){
        pausetta();
        commesso_prende_torta(&pasticceria);
        commesso_vende_torta(&pasticceria);
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

    init_mr(&pasticceria);
    srand(555);


    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        if (i == 0)  pthread_create(&thread[i], NULL, pasticcere, (void *)&taskids[i]);
        if (i == 1)  pthread_create(&thread[i], NULL, commesso, (void *) (&taskids[i]));
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



