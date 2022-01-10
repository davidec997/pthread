
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTIMES 5

typedef enum {false,true} Boolean;
int NUM_CASSIERI , NUM_CLIENTI ;

struct supermercato_t {
    pthread_mutex_t m;
    pthread_cond_t *cassiere, *servito, *cassa;
    int *oggetti_tot;

}supermercato;

void myInit(struct supermercato_t *s)
{
    s->cassa  =     malloc(NUM_CASSIERI * sizeof (pthread_cond_t));
    s->cassiere =   malloc(NUM_CASSIERI * sizeof (pthread_cond_t));
    s->servito =    malloc(NUM_CASSIERI * sizeof (pthread_cond_t));

    pthread_mutex_init(&s->m,NULL);
    s->oggetti_tot = malloc(NUM_CASSIERI* sizeof (int ));

    // s->coda = malloc((4 * NUM_CASSIERI) * sizeof (int));


    for (int i = 0; i < NUM_CASSIERI; i++) {
        pthread_cond_init(&s->cassiere[i],NULL);
        pthread_cond_init(&s->servito[i],NULL);
        pthread_cond_init(&s->cassa[i],NULL);

        s->oggetti_tot[i] = 0;
       // printf( "\t\t\t%d",s->oggetti_tot[i]);
       // printf("\n");
    }
}

void pausetta(void){
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}


int trovaCassiere (struct supermercato_t *s ){
    int index = 0;
    int min = 999;
    for (int i = 1; i < NUM_CASSIERI; i++) {
        //printf("\t\t\t\t%d OGGETTI\n",s->oggetti_tot[i]);
        if (s->oggetti_tot[i] < s->oggetti_tot[index]){
            index = i;
            //min = s->oggetti_tot[i];
        }
    }

    if (index < 0 || index > 10) printf("\tERRORR\n");
    printf("\tECCO IL TUO CAZZO DI INDICE DEL CAZZO %d\n",index);
    return  index;
}

void cliente_pagamento (struct supermercato_t *s, int *pi, int noggetti){
    pthread_mutex_lock(&s->m);
    int index;
    index= trovaCassiere(s);
    printf("[CLIENTE %d]\t HO %d oggetti e vado in coda alla cassa %d\n",*pi,noggetti,index);
    // se il cassiere e' bloccato lo sveglio
    if (s->oggetti_tot[index] == 0) pthread_cond_signal(&s->cassiere[index]);

    printf("SITUA\t");
    for (int i = 0; i < NUM_CASSIERI; i++) printf("%d\t",s->oggetti_tot[i]);
    printf("\n");

    s->oggetti_tot[index] += noggetti;


    printf("[CLIENTE %d]\t ASPETTO CHE IL CASSIERE FINISCA\n",*pi);
    pthread_cond_wait(&s->servito[index],&s->m);

    s->oggetti_tot[index] -= noggetti;
    pthread_mutex_unlock(&s->m);

}

void *cliente(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =0;t<NTIMES;t++){
        sleep(rand()%20 +1);
        printf("[CLIENTE %d]\tSono arrivato al supermercato e sto facendo la spesa\n",*pi);
        int oggetti = (rand()%30) + 1;
        pausetta();
        cliente_pagamento (&supermercato,pi,oggetti);
        printf("[CLIENTE %d]\tHo pagato e vado a casa\n",*pi);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void cassiere_servo_cliente (struct supermercato_t *s , int *pi){
    pthread_mutex_lock(&s->m);
    printf("\t\t\t\t\t\t\tCIAO SONO IL CASSIERE %d\n",*pi);
    if (s->oggetti_tot[*pi] == 0){
        printf("[CASSIERE %d] Mi blocco xk non ho clienti in attesa \n",*pi);
        pthread_cond_wait(&s->cassiere[*pi],&s->m);
    }

    printf("[CASSIERE %d]\t\t Sto servendo un cliente\n",*pi);
    sleep(1);
    pthread_mutex_unlock(&s->m);

}

void cassiere_fine_cliente (struct supermercato_t *s, int *pi) {
    pthread_mutex_lock(&s->m);
    printf("[CASSIERE %d]\t\t SERVITO un cliente\n",*pi);
    pthread_cond_signal(&s->servito[*pi]);
    pthread_mutex_unlock(&s->m);

}

void *cassiere(void *id) {
    int *pi = (int* ) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    // printf("indice %d\n",*pi);
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //  printf("\t\t\t\t\t\t\tCIAO SONO IL CASSIERE %d\n",*pi);

    for(;;){
        sleep(1);
        cassiere_servo_cliente(&supermercato,pi); // pi e' numerocassa
        //servo
        pausetta();
        cassiere_fine_cliente (&supermercato, pi);
    }
    *ptr = *pi;
    pthread_exit((void *) ptr);
}


int main (int argc, char **argv) {
    pthread_t *thread_cassieri;
    pthread_t *thread_clienti;

    int *taskidsCL, *taskidsCA;
    int i;
    int *p;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 3 ) /* Devono essere passati 2 parametri */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_CASSIERI = atoi(argv[2]);
    NUM_CLIENTI = atoi(argv[1]);
    if (NUM_CASSIERI <= 0 || NUM_CLIENTI <= 0)
    {
        sprintf(error, "Errore: Il primo o secondo parametro non e' un numero strettamente maggiore di 0\n");
        perror(error);
        exit(2);
    }

    myInit(&supermercato);

    sleep(2);
    thread_cassieri=(pthread_t *) malloc((NUM_CASSIERI) * sizeof(pthread_t));
    thread_clienti=(pthread_t *) malloc((NUM_CLIENTI) * sizeof(pthread_t));

    if (thread_cassieri == NULL || thread_clienti == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskidsCA = (int *) malloc(NUM_CASSIERI * sizeof(int));
    taskidsCL = (int *) malloc(NUM_CLIENTI * sizeof(int));

    if (taskidsCL == NULL || taskidsCA == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskidsCL o taskidsCA\n");
        exit(4);
    }

    // CREAZIONE PRIMA dei cassieri....
    for (i=0; i < NUM_CASSIERI; i++)
    {
        taskidsCA[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsCL[i]);
        if (pthread_create(&thread_cassieri[i], NULL, cassiere, (void *) (&taskidsCA[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsCL[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread AUTO %i-esimo con id=%lu\n", i, thread_cassieri[i]);
    }
    for (i= 0; i < NUM_CLIENTI ; i++)
    {
        taskidsCL[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsCL[i]);
        if (pthread_create(&thread_clienti[i], NULL, cliente, (void *) (&taskidsCL[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsCL[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread CAMPER %i-esimo con id=%lu\n", i, thread_clienti[i]);
    }

    sleep(2);


    for (i=0; i < NUM_CASSIERI; i++)
    {
        int *ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread_cassieri[i], (void**) & p);
        ris= p;
        printf("Cassiere %d restituisce %d \n", i, *ris);
    }
    for (i=0; i < NUM_CLIENTI; i++)
    {
        int ris;
        pthread_join(thread_clienti[i], (void**) & p);
        ris= *p;
        printf("Cliente %d-esimo restituisce %d \n", i, ris);
    }

    exit(0);
}
