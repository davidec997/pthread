/*
 Un gestore G alloca dinamicamente le risorse R1,...., RM, appartenenti ad un pool di risorse equivalenti,
ai processi P1,..., Pn. Ogni processo puo` richiedere al gestore l'uso di una risorsa, mediante la procedura
Richiesta1, oppure di due risorse contemporaneamente, mediante la procedura Richiesta2. Dopo l'uso il
processo restituisce la (o le) risorse al gestore mediante le procedure Rilascio1 e Rilascio2.
a) utilizzando il meccanismo semaforico, realizzare il gestore G, con le 4 precedenti procedure, e
implementando le seguenti strategie: ogni richiesta deve essere bloccante solo se la (o le) risorse
richieste non sono disponibili e adottando inoltre la seguente priorita`: fra le richieste pendenti
devono essere privilegiate le richieste di due risorse rispetto alle richieste di una sola risorsa e, al loro
interno, le richieste pendenti provenienti dal processo di indice piu` basso (P1 privilegiato rispetto a
P2 ecc).
b) riscrivere la precedente soluzione privilegiando ulteriormente le richieste di due risorse rispetto alle
richieste di una sola risorsa nel seguente modo: una richiesta singola deve essere bloccante anche se
una risorsa e` disponibile qualora ci siano richieste pendenti per due risorse.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NT 10
#define N 2
#define M 4
typedef int R;
typedef enum {false, true} Boolean;
char *scelte[3] = {"SOLO A", "SOLO B", "SIA A CHE B"};
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
sem_t s1,s2,s3;
int ric_A,ric_B,ric_AB;
Boolean A_occ,B_occ;
R risorse [M];
int indice_min;
sem_t sem_i [NT];
int richieste_A[NT], richieste_B[NT], richieste_AB[NT];

void myInit(){
    ric_A = ric_B = ric_AB= 0;
    A_occ = B_occ = false;
    sem_init(&s1,0,0);
    sem_init(&s2,0,0);
    sem_init(&s3,0,0);
    indice_min = 1000;

    for (int i = 0; i < NT; i++) {
        sem_init(&sem_i[i],0,0);
        richieste_A[i]=0;
        richieste_B[i]=0;
        richieste_AB[i]=0;
    }

}

int check_index (int *pi, int richiesta){
    //richiesta 0 --> solo A, 1 --> solo B, 2 --> A e B
    int i;
    switch (richiesta) {
        case 0:
            for ( i = 0; richieste_A == 1; i++)
                return i;
            break;
        case 1:
            for ( i = 0; richieste_B == 1; i++)
                return i;
            break;
        case 2:
            for ( i = 0; richieste_AB == 1; i++)
                return i;
            break;
    }

}

void richiedi_A ( int * pi){

    printf("[PROC %d]\tHA RICHESTO\tA\n",*pi);
    //controllo se A e' libera
    pthread_mutex_lock(&mtx);
    richieste_A[*pi] = 1;
    if (!A_occ && (*pi <= check_index(pi,0))){
        sem_post(&s1);
        A_occ = true;
    } else {
        //if (*pi < indice_min) indice_min = *pi;
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
    richieste_B[*pi] = 1;
    if (!B_occ && (*pi <= check_index(pi,1))){
        sem_post(&s2);
        B_occ = true;
    } else {
       // if (*pi < indice_min) indice_min = *pi;
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
    richieste_A[*pi] = 0;
    printf("[PROC %d]\tRILASCIA\tA\n\n",*pi);
    sveglia();
    pthread_mutex_unlock(&mtx);

}

void rilascia_B (int *pi){
    pthread_mutex_lock(&mtx);
    B_occ = false;
    richieste_B[*pi] = 0;
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
        sleep(rand()%10 + 1);
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


