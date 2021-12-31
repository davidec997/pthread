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

#define POSTI 3
#define NTIMES 10
typedef enum {false,true} Boolean;
int  pazienti_blocc;
int  donatori_blocc;
sem_t s_pazienti, s_donatori,m;
int risorsa;
int lettini_liberi;
Boolean dottore_libero;
char * nomi [2] = {"DONATORE", "PAZIENTE"};

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&s_pazienti, 0, 0);
    sem_init(&s_donatori, 0, 0);
    lettini_liberi = POSTI;
    pazienti_blocc = donatori_blocc = 0;
    dottore_libero = true;
}

void inizio_prelievo( Boolean donatore){
    // donatore --> se 0 e' un donatore con max priorita, se 1 e' paziente normale.
    sem_wait(&m);
    // devo controllare se ci sono lettini liberi e se non ci sono donatori in attesa

    if (!donatore){
        //sono un paziente normale... devo controllare se ci sono donatori in attesa
        if (dottore_libero && lettini_liberi > 0 && donatori_blocc <= 0){
            // il dottore e' libero, ci sono lettini disponibili e non ci sono donatori bloccati
            //posso fare la post previa
            dottore_libero = false;
            lettini_liberi --;
            sem_post(&s_pazienti);
            printf("\t\tSono un %s e HO OTTENUTO IL PERMESSO PER FARE IL PRELIEVO\n",nomi[donatore]);

        } else {
            // una delle 3 condizioni non e'  verificata... mi blocco
            pazienti_blocc ++;
            printf("\t\tSono un %s e MI STO BLOCCANDO\n",nomi[donatore]);

        }

        sem_post(&m);
        sem_wait(&s_pazienti);
        printf("\t\tSono un %s e STO PER FARMI IL PRELIEVO\n",nomi[donatore]);

    } else {
        // sono un donatore
        if (dottore_libero && lettini_liberi > 0){
            dottore_libero = false;
            lettini_liberi --;
            sem_post(&s_donatori);
            printf("\t\tSono un %s e HO OTTENUTO IL PERMESSO PER FARE IL PRELIEVO\n",nomi[donatore]);
        } else {
            donatori_blocc ++;
            printf("\t\tSono un %s e MI STO BLOCCANDO\n",nomi[donatore]);
        }

        sem_post(&m);
        sem_wait(&s_donatori);
        printf("\t\tSono un %s e STO PER FARMI IL PRELIEVO\n",nomi[donatore]);

    }
}

void finePrelievo () {
    sem_wait(&m);
    lettini_liberi ++;
    dottore_libero = true;

    // sveglio donatori bloccati se ce ne sono e poi i pazienti bloccati
    if ( donatori_blocc > 0){
        sem_post(&s_donatori);
        donatori_blocc --;
        lettini_liberi --;
        dottore_libero = false;
    } else if ( pazienti_blocc > 0) {
        sem_post(&s_pazienti);
        pazienti_blocc --;
        lettini_liberi --;
        dottore_libero = false;
    }

    sem_post(&m);
}


void *eseguiCliente(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    Boolean donatore;

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    donatore = rand() % 2;

    for(int t =1;t<NTIMES;t++){
        inizio_prelievo(donatore);
        printf("[THREAD %d]  sono un %s e volgio farmi un prelievo\n",*pi,nomi[donatore]);
        sleep(3);
        finePrelievo();
        printf("[THREAD %d]  sono un %s e ho fatto il prelievo\n",*pi,nomi[donatore]);

    }
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void *eseguiDottore ( void * id) {
    /*int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);*/
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
    srand(555);

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

    //genero il trhead 0 dottore
    pthread_create(&thread[0], NULL, eseguiDottore, (void *) (&taskids[0]));

    for (i=1; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiCliente, (void *) (&taskids[i])) != 0)
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





