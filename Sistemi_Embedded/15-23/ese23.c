#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define N 2
typedef int R;
typedef enum {false, true} Boolean;
char *scelte[3] = {"SOLO A", "SOLO B", "SIA A CHE B"};
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
sem_t s1,s2,s3;
int ric_A,ric_B,ric_AB;
Boolean A_occ,B_occ;
R A;
R B;

void myInit(){
    ric_A = ric_B = ric_AB= 0;
    A_occ = B_occ = false;
    sem_init(&s1,0,0);
    sem_init(&s2,0,0);
    sem_init(&s3,0,0);


}

void richiedi_A ( int * pi){

    printf("[PROC %d]\tHA RICHESTO\tA\n",*pi);
    //controllo se A e' libera
    pthread_mutex_lock(&mtx);
    if (!A_occ){
        sem_post(&s1);
        A_occ = true;
    } else {
        ric_A ++;
        //pthread_mutex_unlock(&mtx);
    }
    pthread_mutex_unlock(&mtx);
    sem_wait(&s1);
    printf("[PROC %d]\tOTTIENE\tA\n",*pi);
    sleep(3);

}

void richiedi_B ( int * pi){
    printf("[PROC %d]\tHA RICHIESTO\tB\n",*pi);
    //controllo se B e' libera
    pthread_mutex_lock(&mtx);
    if (!B_occ){
        sem_post(&s2);
        B_occ = true;
    } else {
        ric_B ++;
        //pthread_mutex_unlock(&mtx);
    }
    pthread_mutex_unlock(&mtx);
    sem_wait(&s2);
    printf("[PROC %d]\tOTTIENE\tB\n",*pi);
    sleep(2);
}

void sveglia(){
    if (ric_AB > 0){
        sem_post(&s3);
        ric_AB --;
    } else if (ric_A > 0){
        sem_post(&s1);
        ric_A --;
    } else if (ric_B > 0){
        sem_post(&s2);
        ric_B --;
    }
}


void rilascia_A (int *pi){
    pthread_mutex_lock(&mtx);
    A_occ = false;
    printf("[PROC %d]\tRILASCIA\tA\n\n",*pi);
    sveglia();
    pthread_mutex_unlock(&mtx);

}

void rilascia_B (int *pi){
    pthread_mutex_lock(&mtx);
    B_occ = false;
    printf("[PROC %d]\tRILASCIA\tB\n\n",*pi);
    sveglia();
    pthread_mutex_unlock(&mtx);
}


void richiedi_AB (int *pi){
    pthread_mutex_lock(&mtx);
    if (A_occ || B_occ){
        ric_AB ++;
    } else {
        sem_post(&s3);
    }
    pthread_mutex_unlock(&mtx);

    sem_wait(&s3);
    richiedi_A(pi);
    richiedi_B(pi);
}

void rilascia_AB (int * pi){
    rilascia_A(pi);
    rilascia_B(pi);
    sveglia();
}

void *eseguiCliente(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int r;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (int i = 0; i < N; i++) {
        sleep(rand()%10 + 3);
        r = rand() % 3;
        printf("[PROC %d]\tVUOLE\t%s\n\n",*pi,scelte[r]);
        switch (r) {
            case 0:
                //printf("GETTING A\n");
                richiedi_A(pi);
                //printf("GETTING A2\n");
                rilascia_A(pi);
               // printf("GETTING 3\n");
                break;
            case 1:
                //printf("GETTING B\n");
                richiedi_B(pi);
               // printf("GETTING B2\n");
                rilascia_B(pi);
                //printf("GETTING B3\n");
                break;
            case 2:
               // printf("GETTING AB\n");
                richiedi_AB(pi);
                //printf("GETTING AB2\n");
                rilascia_AB(pi);
                //printf("GETTING AB3\n");
                break;
        }
    }


    *ptr = 1000+*pi;
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

    myInit();
    srand(555);
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
        if (pthread_create(&thread[i], NULL, eseguiCliente, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    pthread_mutex_destroy(&mtx);
    exit(0);
}


