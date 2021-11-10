//
// Created by davidec on 11/11/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

//STRUTTURE CONDIVISE
typedef struct {
    int voti0;
    int voti1;
}urna;


//VAR GLOBALI
pthread_mutex_t m;
pthread_cond_t votanti,ar;
int mossag1,mossag2;
int  turno;
urna urna_voti;
int NUM_THREADS;

// VAR GLOBALI

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}
void myInit(void)
{
    pthread_cond_init(&ar, NULL);
    pthread_cond_init(&votanti, NULL);
    pthread_mutex_init(&m,NULL);
    urna_voti.voti0 = 0;
    urna_voti.voti1 = 0;
}

int decreta_vincitore(int g1, int g2){
    switch (g1) {
        case 0:
            if(g2 == 0) return 0;
            if(g2 == 1) return 1;
            if(g2 == 2) return 2;
            break;
        case 1:
            if(g2 == 0) return 2;
            if(g2 == 1) return 0;
            if(g2 == 2) return 1;
            break;
        case 2:
            if(g2 == 0) return 1;
            if(g2 == 1) return 2;
            if(g2 == 2) return 0;
            break;
    }
}

void azzeravoti(){
    urna_voti.voti0 = 0;
    urna_voti.voti1 = 0;
}
void *arbitro(void *arg)
{
    char via;
    int *pi = (int *) arg;
    int miovoto;

    while(1){
        pthread_mutex_lock(&m);
        while(turno != 0){
            pthread_cond_wait(&ar, &m);
            if (turno == 1) pthread_cond_broadcast(&votanti);
        }
        //pthread_mutex_lock(&m);
        printf("Vota con 0 oppure con 1\n");
        scanf("%d", &miovoto);
        turno = 1;
        pthread_cond_broadcast(&votanti);
        pthread_mutex_unlock(&m);
        while (turno != 0){
            pthread_cond_wait(&ar, &m);
            if (turno == 1) pthread_cond_broadcast(&votanti);
        }

        turno = 0;
        azzeravoti();
        pthread_mutex_unlock(&m);
    }

}
void vota(int v){
    if (v == 0){
        urna_voti.voti0 ++;
    } else if (v == 1){
        urna_voti.voti1 ++;
    } else{
        printf("Inserito voto no valido\n");
    }

}

int check_votazioni(){
    int rif = (int) (NUM_THREADS/2) + 1;
    if (urna_voti.voti1 + urna_voti.voti0 >= rif) return 1;
    return  0;
}
void *th_votante(void *arg)
{
    //int *pi = (int *) arg;
    int voto;
    while(1) {
        while (check_votazioni()) {

            pthread_cond_wait(&votanti, &m);
            if (turno == 0) pthread_cond_signal(&ar);
        }
        voto = rand() % 2;
        //printf("Il th_votante ha effettuato la mossa \t %s\n", nomi_mosse[mossag1]);
        vota(voto);
        turno = 0;
        pthread_mutex_unlock(&m);
    }
}


int main(int argc, char *argv[]) {
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

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, th_votante, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
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
