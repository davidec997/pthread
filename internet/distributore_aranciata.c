//SEG FAULT
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 20
#define A 10
#define ITER 6
typedef enum {false, true} Boolean;
//araray bool per le risrse
sem_t m, priv_rifornimento,priv_clineti;
int clienti_attesa;
int macchinetta = A;
int aranciate_bevute;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&priv_rifornimento,0,0);
    sem_init(&priv_clineti,0,A);
    clienti_attesa = aranciate_bevute = 0;
}

void ricarica(){
    sem_wait(&priv_rifornimento);
    sem_wait(&m);
    sleep(3);
    macchinetta = A;
    printf("HO APPENA RIFORNITO LA MACCHINETTA...\n");
    sem_post(&m);
    for (int i = 0; i < A; i++) sem_post(&priv_clineti);
    /*if(clienti_attesa){
        while(clienti_attesa){
            sem_post(&priv_clineti);
            clienti_attesa --;
        }
    }*/
}

void bevi_aranciata(int pi){
    if(macchinetta == 0) {
        sem_post(&priv_rifornimento);
        ricarica();
    }
    sem_wait(&priv_clineti);
    sem_wait(&m);
    aranciate_bevute++;
    macchinetta --;
    pausetta();
    printf("Ciao sono %d ho appena bevuto un'aranciata. Con questa siamo a %d aranciate bevute\n",pi,aranciate_bevute);
    sem_post(&m);
}



void *esegui(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int ret = 0;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

   // printf("sto per iniziare thread %d\n",(int*)pi);
    for (int t = 0; t < ITER; t++){
        bevi_aranciata(pi);
        ret++;
    }

    //printf("Thread%d partito: Hello World! Ho come identificatore %lu\n", *pi, pthread_self());
    /* pthread vuole tornare al padre un valore intero, ad es 1000+id */
    *ptr = NULL;
    pthread_exit((void *) ret);
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

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, esegui, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    sleep(5);
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


