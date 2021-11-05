
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

//5 filosofi
#define N 5
#define DELAY 20000000
//struttura per la gestione dello stato e filosofi
//buffer array
int dato =0;
int buffer[N];
sem_t vuoto, pieno, m;

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
    sem_init(&vuoto,0,N);
    sem_init(&pieno,0,0);
}

void *producer(void *arg)
{
    for (int i = 0; i < 100; i++) {
        sem_wait(&vuoto);
        sem_wait(&m);
        dato +=1;
        buffer[i%N] = dato;
        sem_post(&pieno);
        sem_post(&m);
        printf("Il produttore ha prodotto il dato %d e messo nel buffer\n",dato);
    }

}

void *consumer(void *arg)
{
    int dato_prel;
    for (int i = 0; i < 100; i++) {
        sem_wait(&pieno);
        sem_wait(&m);
        dato_prel=buffer[i%N];
        sem_post(&vuoto);
        sem_post(&m);
        printf("Il consumatore ha consumato  il dato %d e tolto dal buffer\n",dato_prel);
    }

}


int main() {
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il sistema */
    myInit();

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
   // pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, producer, 0);
    pthread_create(&p, &a, consumer, 1);

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */\
    sleep(5);
    pthread_join(&p,NULL);
    pthread_join(&p,NULL);

}
