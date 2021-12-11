//FUNZIONA
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define  M 5
#define  NTIMES 5


typedef enum {false, true} Boolean;
Boolean cuoco_sveglio;
pthread_cond_t vuoto,pieno;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int porzioni ;
//per evitare starvation introduco una variabile int col l'indice del processo che indica se il thread i-esimo ha appen mangiato.
//in questo caso verra' deschedulato.
int appena_mangiato;

void myInit(){

    pthread_cond_init(&vuoto,NULL);
    pthread_cond_init(&pieno,NULL);
    cuoco_sveglio = false;
    porzioni = M;
}


void cucina(){
    printf("Sono IL CUOCO  e STO DORMENDO\n");
    pthread_mutex_lock(&m);
    pthread_cond_wait(&vuoto,&m);

    printf("Sono IL CUOCO, SONO STATO SVEGLIATO  E ORA CUCINO\n");
    //addettoAttivo = true;
    sleep(3);
    porzioni = M;
    printf("Sono IL CUOCO E HO FINITO DI CUCINARE\n");
    cuoco_sveglio = false;
    pthread_mutex_unlock(&m);
    pthread_cond_broadcast(&pieno);

}

void *eseguiCuoco(void * id){
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("Ciao sono il thread CUOCO  %d \n",*pi);

    while (1){
        cucina();
    }

    //non arrivo mai qui...
    *ptr = 1000+*pi;
    pthread_exit((void *) ptr);
}

void svegliaCuoco(int id){
    printf("CANNIBALE %d sta svegliando il cuoco \n",id);
    cuoco_sveglio = true;
    pthread_cond_signal(&vuoto);

}


void *eseguiCannibale(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int iterazione,porzioni_prese = 0;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (int i = 0; i < NTIMES; i++) {
         iterazione ++;
        pthread_mutex_lock(&m);

        printf("\nSono il CANNIBALE %d e voglio mangiare per la %d volta\n",*pi,iterazione);

        while( porzioni <= 0){
            // I cannibali che non sono in grado di mangiare svegliano il cuoco
            if (!cuoco_sveglio) svegliaCuoco(*pi);
            pthread_cond_wait(&pieno,&m);
        }

        porzioni --;
        sleep(2); // sleep dentro la SC altrimenti mangiano troppo velocemente
        printf("Thread %d CANNIBALE HA MANGIATO e se ne va\n",*pi);
        porzioni_prese ++;
        printf("\t SITUAZIONE ATTUALE: %d <- PORZIONI DISPONIBILI\n",porzioni);
        pthread_mutex_unlock(&m);
        sleep(2);

    }

    *ptr = porzioni_prese;
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

    myInit();
    srand(555);

    pthread_create(&thread[0], NULL, eseguiCuoco, (void *) (&taskids[0]));
    // addetto
    sleep(3);

    for (i=1; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiCannibale, (void *) (&taskids[i])) != 0)
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
        printf("Pthread %d-esimo restituisce %d <- num di volte che ha mangiato\n", i, ris);
    }

    pthread_mutex_destroy(&m);
    exit(0);
}




