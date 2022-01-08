//PERFETTO
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define POSTI 4     // numero di auto che l'autolavaggio puo lavare contemporaneamente
#define NTIMES 5

typedef enum {false,true} Boolean;
int NUM_CASSIERI, NUM_CLIENTI;

struct supermercato_t {
    sem_t *cassa,*servito, *cassiere;
    sem_t m;
    int *oggetti_cassa;


}supermercato;

void myInit(struct supermercato_t *s)
{
    sem_init(&s->m,0,1);
    s->cassa = malloc(NUM_CASSIERI * sizeof (sem_t));
    s->servito = malloc(NUM_CASSIERI * sizeof (sem_t));
    s->cassiere = malloc(NUM_CASSIERI * sizeof (sem_t));
    s->oggetti_cassa = malloc(NUM_CASSIERI* sizeof (int ));
    for (int i = 0; i < NUM_CASSIERI; i++) {
        sem_init(&s->cassa[i],0,0);
        sem_init(&s->cassiere[i],0,0);
        sem_init(&s->servito[i],0,0);
        s->oggetti_cassa[i] = 0;
    }

}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}


int trovaCassiere (struct supermercato_t *s ){

    int index = -1;
    int min = 999;
    for (int i = 0; i < NUM_CASSIERI; i++) {
        if (s->oggetti_cassa[i] < min)
            index = i;
    }

    if (index == -1) printf("\tERRORR\n");
    return  index;
}

void cliente_pagamento (struct supermercato_t *s, int *pi, int noggetti){
    sem_wait(&s->m);
    int index;
    printf("[CLIENTE %d]\t\toggetti  %d\n",noggetti);
    index = trovaCassiere(s);
    printf("[SERVICE] Situa :");
    for (int i = 0; i < NUM_CASSIERI; i++) printf("\t%d",s->oggetti_cassa[i]);
    printf("\n");

    printf("[CLIENTE %d]\t\t In fila alla cassa %d con oggetti pendenti %d\n",index,s->oggetti_cassa[index]);

    // se il cassiere e'bloccato xk non cerano clienti.. lo sveglio
    if (s->oggetti_cassa[index] == 0)
        sem_post(&s->cassiere[index]);

    sem_post(&s->m);
    sem_wait(&s->cassa[index]);

    sem_wait(&s->m);
    s->oggetti_cassa[index] = noggetti;
    sem_post(&s->m);
    sem_wait(&s->servito[index]);
}

void *cliente(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int lavaggi = 0;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =1;t<NTIMES;t++){
        printf("[CLIENTE %d]\tSono arrivato al supermercato e sto facendo la spesa\n",*pi);
        int oggetti = (rand()%30) + 1;
        cliente_pagamento (&supermercato,pi,oggetti);
        printf("[CLIENTE %d]\tHo pagato e vado a casa\n",*pi);

    }

    *ptr = lavaggi;
    pthread_exit((void *) ptr);
}

void cassiere_servo_cliente (struct supermercato_t *s , int *pi){
    sem_wait(&s->m);
    if (s->oggetti_cassa[*pi] <= 0){
        sem_post(&s->m);
        sem_wait(&s->cassiere[*pi]);
        sem_wait(&s->m);
    }

    // post sulla mia cassa.. il cliente puo lasciare i suoi oggetti
    sem_post(&s->m);
    sem_post(&s->cassa[*pi]);
    pausetta();
    pausetta();
    sem_wait(&s->m);
    printf("[CASSIERE %s]\tHo %d oggetti da battere\n",*pi,s->oggetti_cassa[*pi]);

    sem_post(&s->m);

}

void cassiere_fine_cliente (struct supermercato_t *s, int *pi) {
    sem_wait(&s->m);

    sem_post(&s->servito[*pi]);
    printf("[CASSIERE %s]\tHo battuto gli oggetti... oggetti rimanenti %d\n",*pi,s->oggetti_cassa[*pi]);
    s->oggetti_cassa[*pi] = 0;
    sem_post(&s->m);

}

void *cassiere(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int lavaggi = 0;
    ptr = (int *) malloc(sizeof(int));

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =1;t<NTIMES;t++){
       cassiere_servo_cliente(&supermercato,*pi); // pi e' numerocassa
       //servo
        sleep(1);
       cassiere_fine_cliente (&supermercato, *pi);
    }
    *ptr = lavaggi;
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

    //myInit(&supermercato);

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
    /*for (i=0; i < NUM_CASSIERI; i++)
    {
        taskidsCA[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsCL[i]);
        if (pthread_create(&thread_cassieri[i], NULL, cassiere, (void *) (&taskidsCA[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsCL[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread AUTO %i-esimo con id=%lu\n", i, thread_cassieri[i]);
    }*/
    /*for (i= 0; i < NUM_CLIENTI ; i++)
    {
        taskidsCL[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsCL[i]);
        if (pthread_create(&thread_clienti[i], NULL, cliente, (void *) (&taskidsCL[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsCL[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread CAMPER %i-esimo con id=%lu\n", i, thread_clienti[i]);
    }*/

    sleep(2);


    for (i=0; i < NUM_CASSIERI; i++)
    {
        int *ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread_cassieri[i], (void**) & p);
        ris= p;
        printf("Auto %d-esima restituisce %d <-- numero di volte che e' stata lavata\n", i, *ris);
    }
    for (i=0; i < NUM_CLIENTI; i++)
    {
        int ris;
        pthread_join(thread_clienti[i], (void**) & p);
        ris= *p;
        printf("Camper %d-esimo restituisce %d <-- numero di volte che e' stato lavato\n", i, ris);
    }

    exit(0);
}



