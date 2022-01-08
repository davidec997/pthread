
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTIMES 5

typedef enum {false,true} Boolean;
int NUM_CASSIERI = 5, NUM_CLIENTI =5;

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
        if (s->oggetti_cassa[i] < min)
            index = i;
    }

    if (index == -1) printf("\tERRORR\n");
    return  index;
}


int main()
{
    int i=0;
    pthread_attr_t a;
    pthread_t pa, pb;

    /* inizializzo il mio sistema */
    myInit(&supermercato);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

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