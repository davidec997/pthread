//
// Created by davide on 29/12/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define TORTE_MAX 10
#define TORTE_MIN 3

typedef enum {false, true} Boolean;

struct pasticceria_t {
    pthread_mutex_t m;
    pthread_cond_t fai_torte,stop_torte,cliente_attesa,commesso,stop_commesso;
    int in_fila;
    int torte,torte_pronte;
}pasticceria;

void init_mr (struct pasticceria_t * p){

    pthread_cond_init(&p->fai_torte,NULL);
    pthread_cond_init(&p->stop_torte,NULL);
    pthread_cond_init(&p->cliente_attesa,NULL);


    pthread_mutex_init(&p->m,NULL);
    pthread_cond_init(&p->commesso,NULL);
    pthread_cond_init(&p->stop_commesso,NULL);


    p->torte = p->torte_pronte = 0;
    p->in_fila = 0;

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

    printf("[CLIENTE %d]\t\t VOGLIO UNA TORTA!!\n",*pi);
    p->in_fila ++;

    while (p->torte_pronte <= 0){
        printf("[CLIENTE %d]\t\t IN ATTESA XK SONO FINITE LE TORTE!!\n",*pi);
        pthread_cond_signal(&p->stop_commesso);
        pthread_cond_wait(&p->cliente_attesa,&p->m);
    }

    printf("[CLIENTE %d]\t\t CHIEDO UNA TORTA AL COMMESSO!!\n",*pi);
    //pthread_cond_signal(&p->cliente_attesa);// boooo????
    pthread_cond_wait(&p->commesso,&p->m);

    pausetta();
    p->in_fila --;
    printf("[CLIENTE %d]\t\t HO COMPRATO LA TORTA e VADO A MANGIARLA A CASA\n",*pi);

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

    for(int i =0; i< 10;i++) {
        sleep(rand()%2);
        acquista_torta(&pasticceria,pi);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void inizio_preparazione(struct pasticceria_t *p){
    pthread_mutex_lock(&p->m);
    // il commesso attende che le torte disp siano < di MIN

    while (p->torte >= TORTE_MAX){
        printf("[PASTICCERE]\t\t Non faccio torte perche' ce ne sono ancora %d\n",p->torte);
        pthread_cond_wait(&p->fai_torte,&p->m);
    }

    // se sono qui e' xk i sono meno di MIN torte diponibili
    printf("[PASTICCERE]\t\tINIZIO a fare torte perche' ce ne sono  %d\n",p->torte);

    pthread_mutex_unlock(&p->m);

}

void fine_preparazione(struct pasticceria_t *p){
    pthread_mutex_lock(&p->m);
    p->torte ++;
    printf("[PASTICCERE]\t\tHO APPENA FATTO UNA TORTA.. Ora ce ne sono  %d\n",p->torte);
    pthread_cond_signal(&p->stop_torte);
    pthread_mutex_unlock(&p->m);
}


void prepara_torta(struct pasticceria_t *p){
    sleep(2);

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
    while(p->torte <= TORTE_MIN){
        printf("[COMMESSO]\t\tNON CI SONO ABBASTANZA TORTE  %d\n",p->torte);
        pthread_cond_signal(&p->fai_torte);
        pthread_cond_wait(&p->stop_torte,&p->m);
    }

    while(p->torte_pronte >= TORTE_MAX){
        printf("[COMMESSO]\t\tSMETTO DI INCARTARE TORTE \n");
        //pthread_cond_signal(&p->cliente_attesa);
        pthread_cond_wait(&p->stop_commesso,&p->m);
    }

    printf("[COMMESSO]\t\tIncarto una torta --> TORTE PRONTE %d\n",p->torte_pronte);
    p->torte --;
    p->torte_pronte ++;

    pthread_mutex_unlock(&p->m);
    sleep(2);

}

void commesso_vende_torta(struct pasticceria_t *p){
    pthread_mutex_lock(&p->m);
    if (p->in_fila <= 0){
        printf("[COMMESSO]\t\tNON CI SONO CLIENTI.. CONTINUO AD INCARTARE TORTE\n");
    } else {
        // se c e un cliente in attesa lo servo
        if (p->torte_pronte > 0){
            pthread_cond_signal(&p->cliente_attesa);
            pausetta();
            pthread_cond_signal(&p->commesso);
            p->torte_pronte --;
            printf("[COMMESSO]\t\tHO VENDUTO UNA TORTA\n");
        }
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


