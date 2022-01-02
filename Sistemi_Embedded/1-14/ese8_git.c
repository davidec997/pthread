//
// Created by dada on 02/01/22.
//

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define PROCNUMBER 4
#define CONDIVISENUMBER 2
#define INDEXARGS 99
#define MAXWAITINGTIME 4 // 4 means from 1 to 5 seconds before been activate by clock
#define MAXBUSYTIME 2 // 2 means from 1 to 3 second to operate on a resource
#define NOTIMER 99 //while using a resource timer is unused and set to 99

void pausetta(void){
    unsigned int seconds = (rand()%2+1);
    sleep(seconds);
}
typedef enum {false, true} Boolean ;
//Struttura condivisa
struct Array_t {
    sem_t mutex;
    sem_t privProc[PROCNUMBER];
    sem_t risorseCondivise;
    int risorseUtilizzate[CONDIVISENUMBER];
    int contatori[PROCNUMBER]; //timers
    /* stato del sistema */
}Array;

/* inizializzazione della struttura condivisa */
void init_Array(struct Array_t *a){
    /* semafori e contatori privati */
    sem_init(&a->mutex, 0, 1);
    for (int i=0; i< PROCNUMBER; i++) {
        a->contatori[i] = NOTIMER;
        sem_init(&a->privProc[i], 0, 0);
    }
    for (int i=0; i< CONDIVISENUMBER; i++) {
        a->risorseUtilizzate[i] = 0;
    }
    sem_init(&a->risorseCondivise, 0, CONDIVISENUMBER);
}

int tryAcces(int procNumber){
    sem_wait(&Array.mutex);
    Array.contatori[procNumber] = rand() % MAXWAITINGTIME + 1;
    sem_post(&Array.mutex);

    //Infitite while loop checking for 2 different condition
    while (true) {
        if(sem_trywait(&Array.risorseCondivise) == 0){ //Success
            break;
        }
        if(sem_trywait(&Array.privProc[procNumber]) == 0){ //Success
            break;
        }
    }

    int ritorno;
    sem_wait(&Array.mutex);
    Array.contatori[procNumber] = NOTIMER; //Refresh timer

    if(Array.risorseUtilizzate[0] == 0){
        //Risorsa 0 libera
        Array.risorseUtilizzate[0] = 1;
        ritorno = 0;
    }
    else if(Array.risorseUtilizzate[1] == 0){
        //Risorsa 1 libera
        Array.risorseUtilizzate[1] = 1;
        ritorno = 1;
    }else {
        //Svegliato ma senza risorse
        ritorno = 2;
    }
    sem_post(&Array.mutex);

    return ritorno;
}

void release(int numProc, int tempResource){
    if(tempResource < 2){
        //Se avevo occupato una risorsa la rilascio
        sem_wait(&Array.mutex);
        fprintf(stderr,"Processo: %d RILASCIA la risorsa %d\n", numProc, tempResource);
        Array.risorseUtilizzate[tempResource] = 0;
        sem_post(&Array.risorseCondivise);
        sem_post(&Array.mutex);
    }
}

void checkTick(time_t *lastTime){
    if(time(NULL) - (*lastTime) > 0){
        sem_wait(&Array.mutex);
        fprintf(stderr,"Passato un secondo Orologio attivo, timers: [ ");
        for (int i=0; i< PROCNUMBER; i++) {
            if(Array.contatori[i] != NOTIMER){
                Array.contatori[i]--;
                if(Array.contatori[i] <= 0){
                    sem_post(&Array.privProc[i]);
                }
            }
            fprintf(stderr,"%d ",Array.contatori[i]);
        }
        fprintf(stderr,"]\n");
        sem_post(&Array.mutex);
        (*lastTime)++;
    }
}

void *bodyProc(void *arg){
    int numProc = *((int *) arg);
    for (;;) {
        int tempResource = tryAcces(numProc);
        if(tempResource < 2){
            fprintf(stderr,"Processo: %d accede alla risorsa %d\n", numProc, tempResource);
            pausetta(); // simulazione operazione su risorsa
        }
        else fprintf(stderr,"Processo: %d attivato ma senza risorse \n", numProc);
        release(numProc, tempResource);
    }
    pthread_exit(0);
}

void *bodyOrologio(void *arg){
    time_t lastTime = time(NULL);
    for (;;) {
        checkTick(&lastTime);
    }
    pthread_exit(0);
}

int main(){

    //------------------ VARIABLES AND THREAD ------------------
    pthread_attr_t myattr;
    pthread_t threadOrologio;
    pthread_t threadProc[PROCNUMBER];
    int err;
    void *res;
    int num[INDEXARGS];

    //------------------ INITIALIZATION RANDOM GENERATOR ------------------
    srand(time(NULL));

    //------------------ INITIALIZATION STRUTTURA CONDIVISA ------------------
    init_Array(&Array);

    //------------------ THREAD ATTRIBUTE INITIALIZATION ------------------
    /* initializes the thread attribute */
    pthread_attr_init(&myattr);
    pthread_attr_setdetachstate(&myattr, PTHREAD_CREATE_DETACHED);

    //------------------ THREAD CREATION ------------------
    /* creation and activation of the new thread */
    for (int i=0; i < INDEXARGS; i++) {
        num[i] = i;
    }

    for (int i=0; i< PROCNUMBER; i++) {
        err = pthread_create(&threadProc[i], &myattr, bodyProc, (void*) (&num[i]));
        if(err) fprintf(stderr,"errore creazione thread processo: %u \n", i);
    }

    err = pthread_create(&threadOrologio, &myattr, bodyOrologio, NULL);
    if(err) fprintf(stderr,"errore creazione thread Ricevente\n");

    //------------------ THREAD ATTRIBUTE DESTRUCTION ------------------
    pthread_attr_destroy(&myattr);

    sleep(10); //Sleep 5 seconds before cancellation

    return 0;
}
