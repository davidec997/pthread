//PERFETTO
/*In un autolavaggio vengono lavati due tipi di veicoli: auto
e camper. L’autolavaggio può lavare più auto
contemporaneamente. I camper possono essere lavati solo
se non ci sono né auto né un altro camper in lavaggio, e
hanno priorità sulle auto. Quando arriva un’auto, deve dare
la precedenza agli eventuali camper in attesa.
Si implementi una soluzione usando il costrutto monitor per
modellare l’autolavaggio e i processi per modellare i
veicoli e si descriva la sincronizzazione tra i processi. Nella
soluzione si massimizzi l'utilizzo delle risorse. Si discuta se
la soluzione proposta può presentare starvation e in caso
positivo per quali processi, e si propongano modifiche e/o
aggiunte per evitare starvation.*/

//le auto sono i lettori
//i camper sono gli scrittori

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define POSTI 3
#define NTIMES 10
typedef enum {false,true} Boolean;
int auto_in_lavaggio, auto_bloccate;
int camper_in_lavaggio, camper_bloccati;
sem_t s_auto, s_camper,m;
int risorsa;
int posti_liberi;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&s_auto, 0, 0);
    sem_init(&s_auto, 0, 0);
    posti_liberi = POSTI;
    auto_in_lavaggio = auto_bloccate = camper_in_lavaggio = camper_bloccati = 0;
}

void inizioLettura(){
    sem_wait(&m);
    if(!camper_in_lavaggio && !camper_bloccati  && posti_liberi > 0){
        auto_in_lavaggio ++;
        posti_liberi --;
        sem_post(&s_auto); // post previa
    }else{
        auto_bloccate ++;
    }
    sem_post(&m);
    sem_wait(&s_auto);
}

void svegliaAuto (){
    int auto_da_svegliare = 0;
    if (auto_bloccate > POSTI)
        auto_da_svegliare = POSTI; // se ho piu di NPOSTI auto bloccate, non le sveglio tutte
     else
        auto_da_svegliare = auto_bloccate; // altrimenti posso svegliarle tutte

    while (auto_da_svegliare > 0){
        auto_bloccate--;
        auto_in_lavaggio++;
        sem_post(&s_auto);
    }
}

void fineLettura(){
    sem_wait(&m);
    auto_in_lavaggio --;
    posti_liberi ++;

    if(!auto_in_lavaggio && camper_bloccati > 0){
        sem_post(&s_camper);
        camper_bloccati --;
        camper_in_lavaggio ++;
    }
    sem_post(&m);
}

void inizioScrittura(){
    sem_wait(&m);
    if(!camper_in_lavaggio && !auto_in_lavaggio){
        sem_post(&s_camper);
        camper_in_lavaggio++;
    } else{
        camper_bloccati ++;
    }
    sem_post(&m);
    sem_wait(&s_camper);
}

void fineScrittura(){
    sem_wait(&m);
    camper_in_lavaggio --;
    if(auto_bloccate > 0){
        //svegliaAuto();
        while(auto_bloccate > 0 && posti_liberi > 0) {
            auto_bloccate--;
            auto_in_lavaggio++;
            sem_post(&s_auto);
        }
    } else if (camper_bloccati > 0){
        camper_bloccati--;
        camper_in_lavaggio ++;
        sem_post(&s_camper);
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
        inizioLettura();
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



