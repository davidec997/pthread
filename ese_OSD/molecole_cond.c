//NON VA
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define H 0
#define O 1

#define NTIMES 40
char *nomi_elementi[2] = {"H", "O"};
//sem_t sem_H, sem_O,m;
//int mossag1,mossag2;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_H,cond_O;
//int arbitro_ok,giocatore1ok,giocatore2ok,turno;
int turno, ok_H, ok_O,n_H;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}
void myInit(void)
{
    pthread_cond_init(&cond_H, NULL);
    pthread_cond_init(&cond_O, NULL);

    turno=0, ok_O= 0 , n_H = 0;
    ok_H = 1;
}


void *generatore_H(void *id) {
    printf("Sono Il generatore di H\n");
    //sleep(2);
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));

    for(int i = 0;i< NTIMES;i ++) {
        pthread_mutex_lock(&m);
        if(!ok_H){
            pthread_cond_wait(&cond_H, &m);
        }
        //genero una H
        if (n_H ==1){
            printf("H");
            ok_H = 0;
            ok_O = 1;
            n_H = 0;
            pthread_mutex_unlock(&m);
            pthread_cond_signal(&cond_O);

        } else{
            printf("-H");
            //ok_H = 1;
            //ok_O = 0;
            n_H = 1;
            pthread_mutex_unlock(&m);
            pthread_cond_signal(&cond_H);

        }
    }

    //controllo se e' la seconda h che stampo
    /* if (ok_H == 1){
         printf("H");
         //passo il turno ad O
         turno = 1;
         ok_H =0;
         sem_post(&m);
         sem_post(&sem_O);
     } else {
         //genero 2 H
         printf("-H");
         ok_H = 1;
         sem_post(&m);
         sem_post(&sem_H);
     }*/

    // sleep(2);



}

void *genreratore_O(void *id) {
    printf("Sono Il generatore di O\n");
    //sleep(2);
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));

    for(int i = 0;i< NTIMES/2;i ++) {
        pthread_mutex_lock(&m);
        if(!ok_O){
            pthread_cond_wait(&cond_O,&m);
        }
        printf("O");
        ok_O =0;
        ok_H =1;
        pthread_mutex_unlock(&m);
        pthread_cond_signal(&cond_H);
    }

}


int main() {
    pthread_attr_t a;
    pthread_t p;
    int *taskids;
    taskids = (int *) malloc(2 * sizeof(int));
    taskids[0] = 0;
    taskids[1] = 1;
    /* inizializzo il sistema */
    myInit();

    /* inizializzo i numeri casuali, usati nella funzione pausetta */

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    // pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, generatore_H, &taskids[0]);
    pthread_create(&p, &a, genreratore_O, &taskids[1]);


    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */\
    sleep(10);
    pthread_join(&p,NULL);
    pthread_join(&p,NULL);

    exit(0);

}


