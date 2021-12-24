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

#define N 4
#define NTIMES 5

//typedef enum {false, true} Boolean;

int lettini_liberi, donatori_bloccati, pazienti_bloccati;
sem_t m, medico, pazienti;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&medico, 0, 0);
    sem_init(&pazienti, 0, 0);
    lettini_liberi = N;

}


void effettua_prelievo(){
    sem_wait(&pazienti);
    sem_wait(&m);
    lettini_liberi ++;
    printf("\tSono il Medico e sto facendo un prelievo a un paziente...\n ");
    sleep(1);
    sem_post(&m);
    sem_post(&medico);

}


void richiedi_prelievo(int pi, int * prelievo){
    int * ptr;
    ptr = (int *) malloc(sizeof(int));
    sem_wait(&m);


    if(lettini_liberi <= 0) {
        printf("Il paziente %d ha trovato tutti i letini occupati e se ne va...\n",pi);
        sem_post(&m);
        sleep(3);
    } else {
        lettini_liberi--;
        printf("LETTINI LIBERI %d\n", lettini_liberi);
        sem_post(&pazienti); //sveglio il medico
        *prelievo += 1;
        printf("SONO IL PAZIENTE %d IL MEDICO MI STA FACENDO IL PRELIEVO...\n", pi);
        sem_post(&m);
        sem_wait(&medico);
        sleep(1);
    }
}

void *customerRoutine(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int *taglio = 0;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    printf("Sono il thread paziente %d\n",*pi);

    for (int f =0; f< NTIMES; f++) {
        richiedi_prelievo(*pi, &taglio);
    }

    *ptr = taglio;
    pthread_exit((void *) ptr);

}


void *doctorRoutine(void * id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    int pazienti_serviti =1;
    printf("Sono il thread dottore %d\n",*pi);
    for (;;){
        //sleep(1);
        effettua_prelievo();
        printf("\tSono il dottore e ho effettuato il prelievo a  %d pazienti ...\n", pazienti_serviti);
        pazienti_serviti ++;
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

    myInit();

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

    //genero il trhead 0 barbiere
    pthread_create(&thread[0], NULL, doctorRoutine, (void *) (&taskids[0]));

    for (i=1; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, customerRoutine, (void *) (&taskids[i])) != 0)
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
        /* attendiamo la terminazione di tutti i thread customers */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d  -->numero di volte che si e' tagliato i capelli\n", i, ris);
    }

    //pthread_mutex_destroy(&m);
    exit(0);
}


