//NON VA
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


#define NTIMES 10
sem_t sem_H, sem_O,m;
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
    sem_init(&sem_H,0,2);
    sem_init(&sem_O,0,0);
    sem_init(&m,0,1);
    turno=0, ok_O= 0 , n_H = 0;
    ok_H = 1;
}


void *generatore_H(void *id) {
    //sleep(2);
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    printf("Sono Il generatore di H\n");

    for(int t = 0; t< NTIMES*2; t++) {
        sleep(1);
        sem_wait(&sem_H);
        sem_wait(&sem_H);
        //printf("-H");
        //printf("H");
        write(1,"-H",2);
        write(1,"H",1);
        sem_post(&sem_O);

    }

}

void *genreratore_O(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    printf("Sono Il generatore di O\n");

    for(;;){         //int t = 0; t< NTIMES; t++
        sem_wait(&sem_O);
        write(1,"O",1);
        sem_post(&sem_H);
        sem_post(&sem_H);
        //sleep(1);
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




/*

    for(int i= 0; i< NTIMES; i++) {
        sem_wait(&m);
        if(!ok_H){
            sem_post(&m);
            sem_wait(&sem_H);
        }
        //genero una H
        if (n_H ==1){
            printf("H");
            ok_H = 0;
            ok_O = 1;
            sem_post(&m);
            sem_post(&sem_O);
        } else{
            printf("-H");
            ok_H = 1;
            ok_O = 0;
            sem_post(&m);
            sem_post(&sem_H);
        }
    }
*/
