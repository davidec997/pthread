
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define TORTE_MAX 6
#define TORTE_MIN 3

typedef enum {false, true} Boolean;

struct pasticceria_t {
    pthread_mutex_t m;
    sem_t cliente, compra_torta, s_torte, s_pasticcere, s_commesso;
    int in_fila, in_attesa;
    int torte,torte_pronte;
    Boolean commesso_bloccato;
}pasticceria;

void init_mr (struct pasticceria_t * p){

    pthread_mutex_init(&p->m,NULL);
    sem_init(&p->s_torte,0,0);
    sem_init(&p->s_pasticcere,0,0);
    sem_init(&p->s_commesso,0,0);
    sem_init(&p->compra_torta,0,0);

    p->torte = p->torte_pronte = 0;
    p->in_fila = 0;
    p->in_attesa =0;
    p->commesso_bloccato = true;
}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void acquista_torta (struct pasticceria_t *p, int *pi){

    pthread_mutex_lock(&p->m);
    p->in_fila ++;
    printf("[CLIENTE %d]\t\tVOGLIO una torta.. La chiedo al commesso\n",*pi);
    pthread_mutex_unlock(&p->m);

    //sem_post(&p->compra_torta);
    sem_wait(&p->cliente);
    printf("[CLIENTE %d]\t\tHO COMPRATO LA TORTA\n",*pi);
}

void *cliente (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);}

    for(int i =0; i< 10;i++) {
        sleep((rand()&10));
        acquista_torta(&pasticceria,pi);
    }
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void inizio_preparazione(struct pasticceria_t *p){
    pthread_mutex_lock(&p->m);
    if ( p->torte < TORTE_MAX){
        printf("[PASTICCERE]\tFACCIO UNA TORTA XK LE TORTE  SONO %d\n",p->torte);
        //sem_post(&p->s_pasticcere); // post previa
    }

    pthread_mutex_unlock(&p->m);
    sem_wait(&p->s_pasticcere);

}

void fine_preparazione(struct pasticceria_t *p){
    pthread_mutex_lock(&p->m);
    p->torte ++;
    printf("[PASTICCERE]\t\tHO APPENA FATTO UNA TORTA [Non pronta] .. Ora ce ne sono  %d\n",p->torte);
    if (p->commesso_bloccato) {
        sem_post(&p->s_commesso);
        p->commesso_bloccato = false;
    }
    pthread_mutex_unlock(&p->m);
}


void prepara_torta(struct pasticceria_t *p){
    pthread_mutex_lock(&p->m);
    sleep(2);
    pthread_mutex_unlock(&p->m);
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
        prepara_torta(&pasticceria);
        fine_preparazione(&pasticceria);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void commesso_prende_torta(struct pasticceria_t *p){
    pthread_mutex_lock(&p->m);
    if (p->torte_pronte < TORTE_MAX){
        printf("STO CAAA\n"); // questo non va
        sem_post(&p->s_pasticcere);
    }

    // incarta una torta
    if (p->torte > 0 && p->torte_pronte < TORTE_MAX){
        p->torte --;
        p->torte_pronte ++;
        printf("[COMMESSO]\t\tHo incartato una torta... Torte pronte : %d\n",p->torte_pronte);
    }
    pthread_mutex_unlock(&p->m);
    //sleep(2);

}

void commesso_vende_torta(struct pasticceria_t *p){
    pthread_mutex_lock(&p->m);
    if (p->in_fila > 0  && p->torte_pronte > 0){
        // vendo una torta
        sem_post(&p->cliente);
        p->torte_pronte --;
        p->in_fila --;
        printf("ehii\n");
    }

    pthread_mutex_unlock(&p->m);
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



