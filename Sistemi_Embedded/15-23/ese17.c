//
// Created by davide on 04/01/22.
// NON VA

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

int NUM_THREADS;

typedef enum {false, true} Boolean;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
sem_t aspetta_ris;
int blocc,votati;
int *risultati, *ok;
#define M 5


void myInit(){
    sem_init(&aspetta_ris,0,0);
    risultati = malloc(NUM_THREADS * sizeof (int));
    ok = malloc(NUM_THREADS * sizeof (int));

    for (int i = 0; i < NUM_THREADS; i++) {
        risultati[i] = -1;
        ok[i] = 0;
        votati = 0;
    }
}

int calcola (int *pi){

    int r = rand()%2;
    pthread_mutex_lock(&m);
    risultati[*pi] = r;
    ok[*pi] = 1;
    votati ++;
    pthread_mutex_unlock(&m);
   // sleep(1);
    return r;
}

Boolean esito (int *pi) {
    pthread_mutex_lock(&m);
    int si = 0;
    int no = 0;

    if (votati ==  NUM_THREADS -1) {
        // devo sveglire tutti i thread bloccati
        while (blocc > 0){
            blocc --;
            sem_post(&aspetta_ris);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if ( ok[*pi] && risultati[i] == 1)
            no ++;
        else if (ok[*pi] && risultati[i] == 0)
            si ++;
        else{
            blocc ++;
            printf("MI BLOCCO\n");
            pthread_mutex_unlock(&m);
            sem_wait(&aspetta_ris);
        }
    }
    //sono l'ultimo

    if(votati == NUM_THREADS -1){
        printf("Sono l'ultimo\n");
        printf("\t\tSITUA:");
        for (int i = 0; i < NUM_THREADS; i++) printf("\t%d\t",risultati[i]);
        printf("\n");
        votati = 0;
        for (int i = 0; i < NUM_THREADS; i++){
            risultati[i] = -1;
            ok[i] = 0;
        }
    }

    pthread_mutex_unlock(&m);

    if (si >= M)
        return true;
    else
        return false;
}

void voto(int *pi, int risultato){
    pthread_mutex_lock(&m);
    if (risultati[*pi] == 1){
        printf("[THREAD %d]\t\tRISULTATO GIUSTO\n",*pi,risultati[*pi]);
    } else if (risultati[*pi] == 0){
        printf("[THREAD %d]\t\tRISULTATO ERRRATO\n",*pi,risultati[*pi]);
    } else {
        printf("[THREAD %d]\t\tERROR\n",*pi);
    }
    pthread_mutex_unlock(&m);

}

void correggi ( int *pi){
    risultati[*pi] = 1;
}

void *eseguiUtente(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int r,index;
    //int posti_occupati = 0;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (;;) {
        sleep(2);
        int ris = calcola(pi);
        sleep(1);
        voto(pi, ris);
        if(!esito(pi)) {
            printf("[THREAD %d]\t\tCOMPUTAZIONE SBAGLIATA\n",*pi);
            correggi(pi);
        } else
            printf("[THREAD %d]\t\tCOMPUTAZIONE ESATTO\n",*pi);
        //sleep(2);
    }

    pthread_mutex_unlock(&m);
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

    myInit();
    srand(555);

    // addetto
    sleep(1);

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiUtente, (void *) (&taskids[i])) != 0)
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
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    pthread_mutex_destroy(&m);
    exit(0);
}


