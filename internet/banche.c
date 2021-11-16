
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define N 5
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

int total;

int prelievi [3];
int depositi [3];
int denaro [3];

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void myInit(){
    for (int i = 0; i < 3; ++i) {
        depositi[i] = prelievi [i]= 0;
        denaro[i] = 100;
    }
    total =0;
}


void *banca(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int r,rr;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }


    for(int t =0; t<20; t++){
    sleep(1);
        pthread_mutex_lock(&mtx);
        r = rand();
        rr=rand() % 3;
        if (r %2 == 0){
            //prelievo
            prelievi [rr] ++;
            denaro[rr] -=9;
            printf("Thread%d ha effettuato prelievo... Denaro banca %d --> %d\n\n",(int)id, rr,denaro[rr]);

        }else{
            //deposito
            depositi[rr] ++;
            denaro[rr] +=10;
            printf("Thread%d ha effettuato deposito... Denaro banca %d --> %d\n\n", (int)id, rr,denaro[rr]);

        }
        pausetta();
        pthread_mutex_unlock(&mtx);

    }

    // pthread vuole tornare al padre un valore intero, ad es 1000+id

    pthread_exit((void *) id);
}

void *bancaItalia(void *id)
{
    int *pi = (int *)id;
    int *ptr;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    sleep(4);

    for(int t =0; t<20; t++){
        sleep(4);

        pthread_mutex_lock(&mtx);
        total = 0;
        for (int i = 0; i < 3; ++i) {
            total +=denaro[i];
        }
        printf("Thread%d BANCA D ITALIA. Il totale delle 3 banche e' %d \n\n", (int)id, total);
        printf("Thread%d BANCA D ITALIA. BANCA0 %d  BANCA1 %d   BANCA2 %d \n\n", (int)id, denaro[0],denaro[1],denaro[2]);

        pthread_mutex_unlock(&mtx);
        pthread_yield();

    }

    // pthread vuole tornare al padre un valore intero, ad es 1000+id

    pthread_exit((void *) id);
}

int main (int argc, char *argv[])
{
    pthread_t thread_b, thread_i;
    int i;
    int *p;
    int NUM_THREADS;
    char error[250];
    int banche = 15;

    myInit();
    srand(555);

    for (int j = 0; j < banche; ++j) {
        pthread_create(&thread_b, NULL, banca, j);
    }
    pthread_create(&thread_i, NULL, bancaItalia, 100);

    sleep(50);
    exit(0);
}


