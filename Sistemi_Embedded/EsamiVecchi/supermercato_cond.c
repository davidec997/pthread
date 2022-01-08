
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
    int **coda;

}supermercato;

void myInit(struct supermercato_t *s)
{
    /*s->cassa = s->cassiere = s->servito = malloc(NUM_CASSIERI * sizeof (pthread_cond_t));
    pthread_mutex_init(&s->m,NULL);
    s->oggetti_cassa = malloc(NUM_CASSIERI* sizeof (int ));*/
    s->coda = malloc((4 * NUM_CASSIERI) * sizeof (int));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < NUM_CASSIERI; j++) {
            s->coda[i][j] = 0;
            printf("coda : %d\t",s->coda[i][j]);
        }
    }
    /*for (int i = 0; i < NUM_CASSIERI; i++) {
        pthread_cond_init(&s->cassa[i],NULL);
        pthread_cond_init(&s->cassiere[i],NULL);
        pthread_cond_init(&s->servito[i],NULL);
        s->oggetti_tot[i] = 0;
        s->oggetti_cassa[i] = 0;
    }*/
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
        if (s->oggetti_tot[i] < min)
            index = i;
    }

    if (index == -1) printf("\tERRORR\n");
    return  index;
}

void cliente_pagamento (struct supermercato_t *s, int *pi, int noggetti){

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
        printf("[CLIENTE %d]\tSono arrivato al supermercato e sto facendo la spesa\n",*pi);
        int oggetti = (rand()%30) + 1;
       // cliente_pagamento (&supermercato,pi,oggetti);
        printf("[CLIENTE %d]\tHo pagato e vado a casa\n",*pi);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void cassiere_servo_cliente (struct supermercato_t *s , int *pi){

}

void cassiere_fine_cliente (struct supermercato_t *s, int *pi) {


}

void *cassiere(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =0;t<NTIMES;t++){
        cassiere_servo_cliente(&supermercato,pi); // pi e' numerocassa
        //servo
        sleep(1);
        cassiere_fine_cliente (&supermercato, pi);
    }
    *ptr = *pi;
    pthread_exit((void *) ptr);
}


int main()
{
    int i=0;
    pthread_attr_t a;
    pthread_t pa, pb;

    /* inizializzo il mio sistema */
    myInit(&supermercato);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(time(NULL));

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (i=0; i<NUM_CASSIERI; i++)
        pthread_create(&pb, &a, cassiere, (void *)(i));

    for (i = 0; i < NUM_CLIENTI; i++)
        //pthread_create(&pb, &a, cliente, (void *)(i));

        pthread_attr_destroy(&a);

    sleep(5);

    return 0;
}