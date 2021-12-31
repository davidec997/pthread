//
// Created by davide on 29/12/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


#define N 100 // passeggeri
#define POSTI 4

typedef enum {false, true} Boolean;

struct mr_t {
    pthread_mutex_t m;
    pthread_cond_t vuota,piena,giro,giostra_parti,giostra_pronta,giro_finito;
    int in_fila;
    int posti;
    Boolean vettura_pronta;
}mr;

void init_mr (struct mr_t *mr){
    pthread_cond_init(&mr->vuota,NULL);
    pthread_cond_init(&mr->piena,NULL);
    pthread_cond_init(&mr->giro,NULL);
    pthread_cond_init(&mr->giostra_parti,NULL);
    pthread_cond_init(&mr->giostra_pronta,NULL);
    pthread_cond_init(&mr->giro_finito,NULL);

    pthread_mutex_init(&mr->m,NULL);
    mr->posti = POSTI;
    mr->in_fila = 0;
    mr->vettura_pronta = true;

}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void vai_al_luna_park(int *pi){
    sleep(rand()%10);
    printf("[PASSEGGERO %d]\t\t\te' arrivato al luna park \n",*pi);
    pausetta();
}

void monta_auto(struct mr_t *mr,int *pi){
    // devo aspettare che la vettura sia pronta
    pthread_mutex_lock(&mr->m);
    while (!mr->vettura_pronta || mr-> posti <= 0) {
        printf("[PASSEGGERO %d]\t\t\tSta aspettando che la vettura sia pronta\n", *pi);
        pthread_cond_wait(&mr->giostra_pronta, &mr->m);
    }

    printf("[PASSEGGERO %d]\t\t\tLa GIOSTRA E PRONTA\n",*pi);
    // controllo posti
    if (mr->posti > 0){
        mr->posti --;
        pthread_cond_signal(&mr->piena);
    } /*else if (mr->posti == 0){
        pthread_cond_signal(&mr->giostra_parti); // se la vettura e' piena sveglio la giostra
        printf("[PASSEGGERO %d]\tLA GIOSTRA E' PIENA.. SI PUO PARTIRE --> POSTI: %d\n",*pi,mr->posti);

    }*/
    //pausetta();
    // aspetto che inizi il giro
    printf("[PASSEGGERO %d]\t\tMi sono seduto nella giostra --> POSTI: %d\n",*pi,mr->posti);

    pthread_cond_wait(&mr->giro,&mr->m);

    pthread_mutex_unlock(&mr->m);

}

void paura(int * pi){
    printf("[PASSEGGERO %d]\t\tSTO SULLA GIOSTRA\t\tAAAAAHHHHH\n",*pi);
    sleep(1);
}

void scendi_auto(struct mr_t *mr,int *pi){
    // qui la giostra si e' fermata xk ha completato il giro.. devo scendere
    pthread_mutex_lock(&mr->m);
    pthread_cond_wait(&mr->giro_finito,&mr->m);
    mr->posti ++;
    //pausetta();
    pthread_mutex_unlock(&mr->m);
}

void torna_a_casa(int *pi){
    printf("[PASSEGGERO %d]\tSto tornado a casa...\n",*pi);
}

void *passeggero (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(;;) {
        vai_al_luna_park(pi);
        monta_auto(&mr,pi);
        paura(pi);
        scendi_auto(&mr,pi);
        torna_a_casa(pi);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void vai_al_punto_di_salita_passeggeri(){
    pausetta();
    printf("[GIOSTRA]\t\t\tSONO AL PUNTO DI SALITA\n");
}

void  attendi_passeggeri(struct mr_t *mr){
    // aspetto che siano saliti POSTI passeggeri
    pthread_mutex_lock(&mr->m);

    while (mr->posti > 0){
        pthread_cond_broadcast(&mr->giostra_pronta);
        printf("[GIOSTRA]\t\t\tSONO AL PUNTO DI SALITA ASPETTANDO PASSEGGERI\n");
        pthread_cond_wait(&mr->piena,&mr->m);
    }

    // la vettura e' al completo --> wait su giostra parti
    mr->vettura_pronta = false;
    printf("\n\n[GIOSTRA]\t\t\tNUOVO GIRO\n");
    pthread_cond_broadcast(&mr->giro);

    pthread_mutex_unlock(&mr->m);
}

void giro_della_pista (){
    sleep(3);
}

void svuota_auto (struct mr_t *mr){
    pthread_mutex_lock(&mr->m);
    printf("\n[GIOSTRA]\t\t\tHO TERMINATO IL GIRO\n");

    // resetto i posti disponibili
    //mr->posti = POSTI;
    mr->vettura_pronta = true;
    for (int i = 0; i < POSTI; i++) {
        pthread_cond_signal(&mr->giro_finito);
    }
    pthread_mutex_unlock(&mr->m);
}

void *vettura (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (;;){
        vai_al_punto_di_salita_passeggeri();
        attendi_passeggeri(&mr);
        giro_della_pista();
        svuota_auto(&mr);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv)
{
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 2 ) /* Deve essere passato esattamente un parametro */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0)
    {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
    }

    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    init_mr(&mr);
    srand(555);


    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        if (i == 0)  pthread_create(&thread[i], NULL, vettura, (void *)&taskids[i]);
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, passeggero, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=1; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d \n", i, ris);
    }

    exit(0);
}

