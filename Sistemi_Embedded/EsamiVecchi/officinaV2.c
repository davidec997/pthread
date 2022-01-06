//
// Created by davide on 06/01/22.
//

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTIMES 5

int NUM_CLIENTI, NUM_OPERAI;

typedef enum {false,true} Boolean;
char * elenco_riparazioni = {"GOMMISTA\0","MECCANICO\0", "ELETTRAUTO\0", "CARROZZIERE\0", "POMPISTA\0","TAPPEZZIERE\0","VETRAIO\0"};
int NR = 7;

struct officina_t {
    pthread_mutex_t m;
    pthread_cond_t operaio,clienti,officina_libera,ok_rip;
    int in_attesa;
    //int riparazioni_tot, riparazioni_rimanenti;
    //char **riparazioni;
    Boolean officina_occupata,riparazione_finita;
    //Boolean *operaio_occupato;
    int riparazione;
}officina;


void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void myInit(struct officina_t * o)
{
    pthread_mutex_init(&o->m,NULL);
    pthread_cond_init(&o->officina_libera,NULL);
    pthread_cond_init(&o->ok_rip,NULL);
    pthread_cond_init(&o->operaio,NULL);
    pthread_cond_init(&o->clienti,NULL);


    o->officina_occupata = true;
    o->riparazione_finita = false;
    o->in_attesa = 0;
    o->officina_occupata = false;
    o->riparazione_finita = false;

    o->riparazione = -1;
}

void attesaCliente ( struct  officina_t *o, int *pi, int riparazione){
    pthread_mutex_lock(&o->m);

    while (o->riparazione != riparazione){
        printf("[OPERAIO %d]\t\t\tATTENDE CLIENTE\n",*pi);
        pthread_cond_wait(&o->operaio, &o->m);
    }

    // ok un cliente vuole la mia riparazione
    printf("[OPERAIO %d]\t\t\t****INIZIO RIPARAZIONE %d*****\n",*pi, riparazione);
    pausetta();
    pthread_mutex_unlock(&o->m);
}

void fineServizio (struct officina_t *o, int *pi, int riparazione){
    pthread_mutex_lock(&o->m);

    o->riparazione_finita = true;
    printf("[OPERAIO %d]\t\t\t#####HA FINITO RIPARAZIONE %d#####\n",*pi, riparazione);

    pthread_mutex_unlock(&o->m);
    pthread_cond_signal(&o->ok_rip);

}

void *eseguiOperaio(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //printf("[OPERAIO %d] SONO UN %s\n",*pi,officina.riparazioni[*pi]);

    for(;;){
        attesaCliente(&officina,pi,*pi);
        //printf("[OPERAIO %d] SONO UN %s SONO A LAVORO\n",*pi,&officina.riparazioni[*pi]);
        sleep(2);
        fineServizio(&officina,pi,*pi);
        sleep(2);
    }

    *ptr = 0;
    pthread_exit((void *) ptr);
}

void arrivo (struct  officina_t *o, int *pi, int riparazione){
    pthread_mutex_lock(&o->m);
    while (o->officina_occupata){
        printf("[CLIENTE %d]\t\t IN ATTESA PERCHE' L'OFFICINA E' OCCUPATA\n",*pi);
        pthread_cond_wait(&o->officina_libera,&o->m);
    }

    o->officina_occupata = true;
    printf("\n[CLIENTE %d]\t\t------>RICHIEDE RIPARAZIONE %d\n",*pi, riparazione);
    o->riparazione = riparazione;

    pthread_cond_broadcast(&o->operaio);

    pthread_mutex_unlock(&o->m);
}

void attendi_riparazione (struct officina_t *o, int *pi, int riparazione){
    pthread_mutex_lock(&o->m);

    while(!o->riparazione_finita){
        printf("[CLIENTE %d]\t\tASPETTA IL COMPLETAMENTO DELLA PRIPARAZIONE %d\n",*pi, o->riparazione);
        pthread_cond_wait(&o->ok_rip,&o->m);
    }

    printf("[CLIENTE %d]\t\t PRIPARAZIONE %d TERMINATA <---------\n",*pi, o->riparazione);
    // sveglio il prossimo cliente

    o->officina_occupata = false;
    pthread_mutex_unlock(&o->m);

    pthread_cond_signal(&o->officina_libera);
}

void *eseguiCliente(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int riparazione;
    ptr = (int *) malloc(sizeof(int));

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(;;){
        sleep(rand()%8);
        printf("[CLINETE %d]\tSono arrivato in Officina\n",*pi);
        riparazione = rand() % NUM_OPERAI;
        arrivo(&officina,pi,riparazione);
        printf("[CLINETE %d]\t\t\tIN RIPARAZIONE  %d\n",*pi, riparazione);
        attendi_riparazione(&officina,pi,riparazione);
        //sleep(rand()%3 + 1);
    }
    *ptr = riparazione;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv) {
    pthread_t *thread_clienti;
    pthread_t *thread_operai;

    int *taskidsA, *taskidsC;
    int i;
    int *p;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 3 ) /* Devono essere passati 2 parametri */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_CLIENTI = atoi(argv[1]);
    NUM_OPERAI = atoi(argv[2]);
    if (NUM_CLIENTI <= 0 || NUM_OPERAI <= 0)
    {
        sprintf(error, "Errore: Il primo o secondo parametro non e' un numero strettamente maggiore di 0\n");
        perror(error);
        exit(2);
    }

    myInit(&officina);
    srand(time(NULL));

    thread_clienti=(pthread_t *) malloc((NUM_CLIENTI) * sizeof(pthread_t));
    thread_operai=(pthread_t *) malloc((NUM_OPERAI) * sizeof(pthread_t));

    if (thread_clienti == NULL || thread_operai == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskidsA = (int *) malloc(NUM_CLIENTI * sizeof(int));
    taskidsC = (int *) malloc(NUM_OPERAI * sizeof(int));

    if (taskidsA == NULL || taskidsC == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskidsA o taskidsC\n");
        exit(4);
    }

    // CREAZIONE PRIMA DEGLI OPERAI....
    for (i= 0; i < NUM_OPERAI ; i++)
    {
        taskidsC[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsA[i]);
        if (pthread_create(&thread_operai[i], NULL, eseguiOperaio, (void *) (&taskidsC[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsA[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread CAMPER %i-esimo con id=%lu\n", i, thread_operai[i]);
    }

    for (i=0; i < NUM_CLIENTI; i++)
    {
        taskidsA[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsA[i]);
        if (pthread_create(&thread_clienti[i], NULL, eseguiCliente, (void *) (&taskidsA[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsA[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread AUTO %i-esimo con id=%lu\n", i, thread_clienti[i]);
    }


    sleep(2);


    for (i=0; i < NUM_CLIENTI; i++)
    {
        int *ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread_clienti[i], (void**) & p);
        ris= p;
        printf("Cliente %d-esima restituisce %d <-- numero di riparazioni richieste\n", i, *ris);
    }
    for (i=0; i < NUM_OPERAI; i++)
    {
        int ris;
        pthread_join(thread_operai[i], (void**) & p);
        ris= *p;
        printf("Operaio %d-esimo restituisce %d <-- numero di riparazioni effettuate\n", i, ris);
    }

    exit(0);
}
