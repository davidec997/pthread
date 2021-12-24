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
int auto_in_lavaggio, pazienti_blocc;
int camper_in_lavaggio, donatori_blocc;
sem_t s_pazienti, s_donatori,m;
int risorsa;
int lettini_liberi;
Boolean dottore_libero;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&s_pazienti, 0, 0);
    sem_init(&s_pazienti, 0, 0);
    lettini_liberi = POSTI;
    auto_in_lavaggio = pazienti_blocc = camper_in_lavaggio = donatori_blocc = 0;
    dottore_libero = true;
}

void inizio_prelievo( Boolean tipo){
    // tipo --> se 0 e' un donatore con max priorita, se 1 e' paziente normale.
    sem_wait(&m);
    // devo controllare se ci sono lettini liberi e se non ci sono donatori in attesa
    if (tipo){

    }

    if( dottore_libero && lettini_liberi > 0){
        if (tipo){
            // se sono un paziente normale devo dare precendeza ai donatori
            if ( donatori_blocc > 0){

            }
        }
        auto_in_lavaggio ++;
        lettini_liberi --;
        sem_post(&s_pazienti); // post previa
    }else{
        pazienti_blocc ++;
    }
    sem_post(&m);
    sem_wait(&s_pazienti);
}

void svegliaAuto (){
    int auto_da_svegliare = 0;
    if (pazienti_blocc > POSTI)
        auto_da_svegliare = POSTI; // se ho piu di NPOSTI auto bloccate, non le sveglio tutte
    else
        auto_da_svegliare = pazienti_blocc; // altrimenti posso svegliarle tutte

    while (auto_da_svegliare > 0){
        pazienti_blocc--;
        auto_in_lavaggio++;
        sem_post(&s_pazienti);
    }
}

void fineLettura(){
    sem_wait(&m);
    auto_in_lavaggio --;
    lettini_liberi ++;

    if(!auto_in_lavaggio && donatori_blocc > 0){
        sem_post(&s_donatori);
        donatori_blocc --;
        camper_in_lavaggio ++;
    }
    sem_post(&m);
}

void inizioScrittura(){
    sem_wait(&m);
    if(!camper_in_lavaggio && !auto_in_lavaggio){
        sem_post(&s_donatori);
        camper_in_lavaggio++;
    } else{
        donatori_blocc ++;
    }
    sem_post(&m);
    sem_wait(&s_donatori);
}

void fineScrittura(){
    sem_wait(&m);
    camper_in_lavaggio --;
    if(pazienti_blocc > 0){
        //svegliaAuto();
        while(pazienti_blocc > 0 && lettini_liberi > 0) {
            pazienti_blocc--;
            auto_in_lavaggio++;
            sem_post(&s_pazienti);
        }
    } else if (donatori_blocc > 0){
        donatori_blocc--;
        camper_in_lavaggio ++;
        sem_post(&s_donatori);
    }

    sem_post(&m);
}

void *eseguiCamper(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =1;t<NTIMES;t++){
        inizioScrittura();
        //risorsa += 1;
        printf("[CAMPER %d]  mi sto lavando\n",*pi);
        sleep(6);
        fineScrittura();
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void *eseguiAuto(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for(int t =1;t<NTIMES;t++){
        inizio_prelievo();
        printf("[AUTO %d]  mi sto lavando\n",*pi);
        sleep(3);
        fineLettura();
    }
    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv)
{
    pthread_t *thread_auto;
    pthread_t *thread_camper;

    int *taskidsA, *taskidsC;
    int i;
    int *p;
    int NUM_AUTO, NUM_CAMPER;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 3 ) /* Devono essere passati 2 parametri */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_AUTO = atoi(argv[1]);
    NUM_CAMPER = atoi(argv[2]);
    if (NUM_AUTO <= 0 || NUM_CAMPER <= 0)
    {
        sprintf(error, "Errore: Il primo o secondo parametro non e' un numero strettamente maggiore di 0\n");
        perror(error);
        exit(2);
    }

    myInit();

    thread_auto=(pthread_t *) malloc((NUM_AUTO) * sizeof(pthread_t));
    thread_camper=(pthread_t *) malloc((NUM_CAMPER) * sizeof(pthread_t));

    if (thread_auto == NULL || thread_camper == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskidsA = (int *) malloc(NUM_AUTO * sizeof(int));
    taskidsC = (int *) malloc(NUM_CAMPER * sizeof(int));

    if (taskidsA == NULL || taskidsC == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskidsA o taskidsC\n");
        exit(4);
    }

    // CREAZIONE PRIMA DELLE AUTO....
    for (i=0; i < NUM_AUTO; i++)
    {
        taskidsA[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsA[i]);
        if (pthread_create(&thread_auto[i], NULL, eseguiAuto, (void *) (&taskidsA[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsA[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread AUTO %i-esimo con id=%lu\n", i, thread_auto[i]);
    }
    for (i= 0; i < NUM_CAMPER ; i++)
    {
        taskidsC[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskidsA[i]);
        if (pthread_create(&thread_camper[i], NULL, eseguiCamper, (void *) (&taskidsC[i])) != 0)
        {
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsA[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread CAMPER %i-esimo con id=%lu\n", i, thread_camper[i]);
    }

    sleep(2);

    for (i=0; i < NUM_AUTO; i++)
    {
        int *ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread_auto[i], (void**) & p);
        ris= p;
        printf("Pthread  %d-esimo restituisce %d\n", i, *ris);
    }
    for (i=0; i < NUM_CAMPER; i++)
    {
        int ris;
        pthread_join(thread_camper[i], (void**) & p);
        ris= *p;
        printf("Pthread Scrittore %d-esimo restituisce %d\n", i, ris);
    }

    exit(0);
}




