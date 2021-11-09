//VERSIONE CHE USA MUTEX E COND VAR  --> funziona!!!

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

//5 filosofi
#define N 5
#define DELAY 20000000
//struttura per la gestione dello stato e filosofi

int filosofi = N;
int stato [N];

// def sequenze
#define THINKING 0
#define HUNGRY 1
#define EATING 2

//mutex e cond var
pthread_mutex_t m;
pthread_cond_t cond_filo[N];

void myInit(void)
{
    pthread_mutex_init(&m,NULL);
    for (int i =0; i<N; i++) pthread_cond_init(&cond_filo[i],NULL);
    for(int t =0;t<N;t++) stato[t] = THINKING;
}

void think(int index){
    pthread_mutex_lock(&m);
    printf("Filosofo %d sta pensando...\n",index);
    stato[index]= HUNGRY;
    pthread_mutex_unlock(&m);
    for(int t = 0;t< DELAY; t++);

}

int puoiMangiare(int indice){
    return ((stato[indice] == HUNGRY) && (stato[(indice +1) % N] != EATING) && (stato[(indice + 4) % N] != EATING));
}

void preEat(int index){
    pthread_mutex_lock(&m);
    while (!puoiMangiare(index)){
        pthread_cond_wait(&cond_filo[index], &m);
    }
    stato[index] = EATING;
    printf("Filosofo %d ha ottenuto le bacchette [%d] e [%d]\n",index, ((index) % N),((index +4)% N));

    pthread_mutex_unlock(&m);
}

void postEat(int index){
    pthread_mutex_lock(&m);
    stato[index] = THINKING;
    printf("Filosfo %d ha finito di mangiare...\n",index);
    //se sono bloccati devo svegliare i vicini di dx e sx
    if(puoiMangiare((index + 1) % N)){
        pthread_cond_signal(&cond_filo[(index + 1)% N]);
       // stato[(index + 1) % N]= EATING;
    } else if(puoiMangiare((index + 4) % N)) {
        pthread_cond_signal(&cond_filo[(index + 4)% N]);
        //stato[(index + 4) % N]= EATING;
    }
    pthread_mutex_unlock(&m);
}

void eat(int index){
    printf("Ciao sono il filo %d e sto magnando...\n",index);
    for (int i = 0; i < DELAY; ++i);
}

void *dinner(void *arg)
{
    int index = (int)arg;
    printf("Ciao sono il filosofo %d e sono stato creato \n",index);
    for (;;) {
        think(index);
        preEat(index);
        eat(index);
        postEat(index);
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
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, dinner, 0);
    pthread_create(&p, &a, dinner, 1);
    pthread_create(&p, &a, dinner, 2);
    pthread_create(&p, &a, dinner, 3);
    pthread_create(&p, &a, dinner, 4);

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(10);

}

