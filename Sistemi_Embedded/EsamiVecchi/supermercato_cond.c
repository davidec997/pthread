
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTIMES 5

typedef enum {false,true} Boolean;
int NUM_CASSIERI = 5, NUM_CLIENTI =5;

struct supermercato_t {
    pthread_mutex_t m;
    pthread_cond_t *cassiere,*servito,* cassa;
    int *oggetti_cassa,*oggetti_tot;
    int coda[4][5];
    int head[4], tail[4];

}supermercato;

void myInit(struct supermercato_t *s)
{
    s->cassa = s->cassiere = s->servito = malloc(NUM_CASSIERI * sizeof (pthread_cond_t));
    pthread_mutex_init(&s->m,NULL);
    s->oggetti_cassa = malloc(NUM_CASSIERI* sizeof (int ));
    s->oggetti_tot = malloc(NUM_CASSIERI* sizeof (int ));

    // s->coda = malloc((4 * NUM_CASSIERI) * sizeof (int));
    for (int i = 0; i < 4; i++) {
        s->head[i]= 0;
        s->tail[i] = 0;
        for (int j = 0; j < NUM_CASSIERI; j++) {
            s->coda[i][j] = 0;
           // printf("coda : %d\t",s->coda[i][j]);
        }
    }
    for (int i = 0; i < NUM_CASSIERI; i++) {
        pthread_cond_init(&s->cassa[i],NULL);
        pthread_cond_init(&s->cassiere[i],NULL);
        pthread_cond_init(&s->servito[i],NULL);
        s->oggetti_tot[i] = 0;
        s->oggetti_cassa[i] = 0;
    }
}

void pausetta(void){
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}


int trovaCassiere (struct supermercato_t *s ){
    int index = -1;
    int min = 999;
    for (int i = 0; i < NUM_CASSIERI; i++) {
        //printf("\t\t\t\t%d OGGETTI\n",s->oggetti_tot[i]);
        if (s->oggetti_tot[i] < min){
            index = i;
            min = s->oggetti_tot[i];
        }
    }

    if (index < 0 || index > 10) printf("\tERRORR\n");
    return  index;
}

void cliente_pagamento (struct supermercato_t *s, int *pi, int noggetti){
    pthread_mutex_lock(&s->m);
    int index = trovaCassiere(s);
    printf("[CLIENTE %d]\t\tSi accoda in cassa %d che ha solo %d oggetti\n",index,s->oggetti_tot[index]);
    s->oggetti_tot[index] += noggetti;

    // aspetto l'ok dal casssiere
    pthread_cond_signal(&s->cassiere[index]);
    printf("[CLIENTE %d]\t\tok dal cassiere\n",index);

    pthread_cond_wait(&s->cassa[index],&s->m);
    s->oggetti_cassa[index] = noggetti;

    pthread_cond_wait(&s->servito[index],&s->m);
    printf("[CLIENTE %d]\t\tservito cassiere\n",index);

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
        pausetta();
        printf("[CLIENTE %d]\tSono arrivato al supermercato e sto facendo la spesa\n",*pi);
        int oggetti = (rand()%30) + 1;
        cliente_pagamento (&supermercato,pi,oggetti);
        printf("[CLIENTE %d]\tHo pagato e vado a casa\n",*pi);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void cassiere_servo_cliente (struct supermercato_t *s , int *pi){
    pthread_mutex_lock(&s->m);
    if(s->oggetti_tot[*pi] <= 0){
        pthread_cond_wait(&s->cassiere[*pi],&s->m);
    }

    pausetta();
    pthread_cond_signal(&s->cassa[*pi]);
    pthread_mutex_unlock(&s->m);

}

void cassiere_fine_cliente (struct supermercato_t *s, int *pi) {
    pthread_mutex_lock(&s->m);
    printf("[CASSIERE %d] Ho %d oggetti da battere\n",*pi,s->oggetti_cassa);
    sleep(1);
    s->oggetti_tot[*pi] -= s->oggetti_cassa[*pi];
    s->oggetti_cassa[*pi] = 0;
    pausetta();
    pthread_cond_signal(&s->servito[*pi]);
    printf("[CASSIERE %d]\t\t\tHo servito con amore\n",*pi);
    pthread_mutex_unlock(&s->m);

}

void *cassiere(void *id) {
    int *pi = (int* ) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    printf("indice %d\n",pi);
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =0;t<NTIMES;t++){
        cassiere_servo_cliente(&supermercato,*pi); // pi e' numerocassa
        //servo
        sleep(1);
        cassiere_fine_cliente (&supermercato,* pi);
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



/*

int main()
{
    int i=0;
    pthread_attr_t a;
    pthread_t pa, pb;

    */
/* inizializzo il mio sistema *//*

    myInit(&supermercato);

    */
/* inizializzo i numeri casuali, usati nella funzione pausetta *//*

    srand(time(NULL));

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (i=0; i<NUM_CASSIERI; i++)
        //pthread_create(&pb, &a, cassiere, (void *)(i));

    for (i = 0; i < NUM_CLIENTI; i++)
        pthread_create(&pb, &a, cliente, (void *)(i));

        pthread_attr_destroy(&a);

    sleep(5);

    return 0;
}*/
