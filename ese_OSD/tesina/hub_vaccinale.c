//
// Created by davidec on 26/12/21.
//

/*
 In un centro per il prelievo del sangue lavora un medico
che ha a disposizione L lettini. Le persone che effettuano il
prelievo si dividono in due categorie: donatori e pazienti.
Ogni persona può iniziare il prelievo solo quando è
disponibile il medico e c’è almeno un lettino vuoto, altrimenti
aspetta. Dopo che il medico ha iniziato il prelievo, la
persona aspetta che il medico finisca il prelievo. Terminato il
prelievo, dopo essersi ripresa, la persona libera il lettino.
Nella soluzione si tenga presente che i donatori hanno la
precedenza sui pazienti.
Si implementi una soluzione usando il costrutto monitor per
modellare il centro prelievi e i processi per modellare il
medico e le persone e si descriva la sincronizzazione tra i
processi. Nella soluzione si massimizzi l’utilizzo delle
risorse.
 * */
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define DOTTORI 3
#define NTIMES 3

typedef enum {false,true} Boolean;
int  dosi_blocc [3];
sem_t prima_dose, seconda_dose, terza_dose, m;
sem_t s_dosi [3];
int dottori_liberi;
char *elenco_dosi [3] = {"PRIMA", "SECONDA", "TERZA"};

void myInit(void)
{
    sem_init(&m,0,1);
    for (int i = 0; i < 3; ++i) {
        sem_init(&s_dosi[i], 0, 0);
        dosi_blocc [i] = 0;
    }

    dottori_liberi = DOTTORI;
}

void okVaccino ( int * pi, int dose){
    dottori_liberi --;
    sem_post(&s_dosi[dose]);
    printf("[THREAD %d]\tOk per la %s dose.\n", *pi, elenco_dosi[dose]);
    printf("\n[SERVICE]\t\t Dottori liberi: %d\n", dottori_liberi);

}

void notOkVaccino ( int * pi, int dose){
    dosi_blocc[dose] ++;
    printf("[THREAD %d]\tBloccato per la %s dose.\n", *pi,elenco_dosi[dose]);
    printf("\n[SERVICE]\t\tBloccati per la %s dose: %d\n",elenco_dosi[dose], dosi_blocc[dose]);
}

void vaccino (int * pi,int dose){
    // le persone che devono fare la prima dose hanno la precedenza su chi deve fare la seconda, e chi deve fare la seconda ha precedenza su quelli che devono fare la terza
    sem_wait(&m);
    switch (dose) {
        case 0:
            if (dottori_liberi > 0 )  // posso vaccinarmi
                okVaccino(pi,dose);
            else                     // non posso vaccinarmi
                notOkVaccino(pi, dose);
            break;
        case 1:
            if (dottori_liberi > 0 && dosi_blocc[0] == 0)
                okVaccino(pi,dose);
            else
                notOkVaccino(pi,dose);
            break;
        case 2:
            if (dottori_liberi > 0 && dosi_blocc[0] == 0 && dosi_blocc[1] == 0)
                okVaccino(pi, dose);
            else
                notOkVaccino(pi, dose);
            break;
    }

    sem_post(&m);
    sem_wait(&s_dosi[dose]);

}

void fine_vaccino ( int * pi){
    // quando qualcuno ha finito di fare il vaccino deve svegliare chi e' in attesa e ha precedenza
    sem_wait(&m);
    dottori_liberi ++;

    for (int i = 0; i < 3; ++i) {
        if (dosi_blocc[i] > 0){
            okVaccino(pi, i);
            dosi_blocc[i] --;
            printf("[SERVICE]\t\tHo svegliato qualcuno che deve fare la %s dose\n\n",elenco_dosi[i]);
            break;
        }
    }
    sem_post(&m);
}


void *eseguiVaccini(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));

    int dose = 0;

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //  ogni persona deve fare 3 dosi
    for(int t = 0; t<NTIMES; t++){
        sleep(rand() % 8);

        printf("[THREAD %d]\tDevo fare la %s dose del vaccino\n",*pi,elenco_dosi[dose]);
        vaccino(pi, dose);
        sleep(3);

        printf("[THREAD %d]\tMi sto vaccinando....\n",*pi);
        fine_vaccino(pi);
        printf("[THREAD %d]\tHo fatto la %s dose del vaccino\n\n",*pi,elenco_dosi[dose]);
        dose ++;
    }

    *ptr = dose;
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

    myInit();
    srand(time(NULL));

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

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiVaccini, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread customers */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d  --> numero di volte che si e' vaccinato\n", i, ris);
    }

    //pthread_mutex_destroy(&m);
    exit(0);
}



