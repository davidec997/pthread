//PERFETTO

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTIMES 5

typedef enum {false,true} Boolean;
int NUM_OP,NUM_CLI;
char * elenco_riparazioni = {"GOMMISTA\0","MECCANICO\0", "ELETTRAUTO\0", "CARROZZIERE\0", "POMPISTA\0","TAPPEZZIERE\0","VETRAIO\0"};
int NR = 7;

struct officina_t {
    pthread_mutex_t m;
    pthread_cond_t *operai,*clienti,officina_libera,ok_rip;
    int in_attesa;
    int riparazioni_tot, riparazioni_rimanenti;
    char **riparazioni;
    Boolean officina_occupata,riparazione_finita;
    Boolean *operaio_occupato;
}officina;


void myInit(struct officina_t * o)
{
    pthread_mutex_init(&o->m,NULL);
    pthread_cond_init(&o->officina_libera,NULL);
    pthread_cond_init(&o->ok_rip,NULL);

    o->riparazioni = malloc(NUM_OP * 20);
    for (int i = 0; i < NUM_OP; ++i) {
        pthread_cond_init(&o->operai[i],NULL);
        o->riparazioni[i] = elenco_riparazioni[rand()% NR];
    }
    for (int t = 0; t < NUM_CLI; t ++) pthread_cond_init(&o->clienti[t],NULL);

    o->riparazioni_tot = o->riparazioni_rimanenti = 0;
    o->officina_occupata = true;
    o->riparazione_finita = false;


    o->operaio_occupato = malloc(NUM_OP * sizeof (int ));
    for (int f = 0; f < NUM_OP; f++) o->operaio_occupato[f] = false;

}

void attesaCliente(struct officina_t *o , int *pi){
    pthread_mutex_lock(&o->m);
    while ( o->in_attesa <= 0){
        printf("[OPERAIO %d %s]\tSONO IN ATTESA DI CLIENTI\n",*pi,o->riparazioni[*pi]);
        pthread_cond_wait(&o->clienti[*pi],&o->m);
    }

    //c'e un cliente
    //blocco l officina
    o->officina_occupata = true;
    o->operaio_occupato[*pi] = true;
    printf("[OPERAIO %d %s]\tINIZIO INTERVENTO\n",*pi,o->riparazioni[*pi]);
   // sleep(2);


    pthread_mutex_unlock(&o->m);

}

void fineServizio (struct officina_t *o , int * pi){
    pthread_mutex_lock(&o->m);

    o->officina_occupata = false;
    o->operaio_occupato[*pi] = false;
    printf("[OPERAIO %d %s]\tFINITO INTERVENTO\n",*pi,o->riparazioni[*pi]);
    // sleep(2);

    pthread_mutex_unlock(&o->m);

    pthread_cond_signal(&o->officina_libera);
    pthread_cond_signal(&o->operai[*pi]);
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
        attesaCliente(&officina,pi);
        //printf("[OPERAIO %d] SONO UN %s SONO A LAVORO\n",*pi,&officina.riparazioni[*pi]);
        sleep(2);
        fineServizio(&officina,pi);
    }

    *ptr = 0;
    pthread_exit((void *) ptr);
}

void arrivo (struct officina_t *o, int *pi, int riparazione){

    pthread_mutex_lock(&o->m);
   // printf("[CLINETE %d]\tDevo effettuare intervento da : %s\n",*pi,o->riparazioni[riparazione]);

    while (o->officina_occupata){
        printf("[CLINETE %d]\tOFFICINA OCCUPATA... ATTENDO\n",*pi);
        pthread_cond_wait(&o->officina_libera,&o->m);
    }

    // segnalo che c'e' un cliente
    o->in_attesa ++;
    pthread_cond_signal(&o->clienti[*pi]);

    printf("[CLINETE %d]\tSONO ENTRATO IN OFFICINA\n",*pi);

    printf("[SERVICE] SIUAZIONE :");
    for (int i = 0; i < NUM_OP; i++) printf("\t[%d]\t",o->operaio_occupato[i]);
    printf("\n");

    while (o->operaio_occupato[riparazione]){
      //  printf("[CLINETE %d]\tL'OPERAIO PER LA MIA RIPARAZIONE %s E' OCCUPATO...\n",*pi,elenco_riparazioni[riparazione]);
        pthread_cond_wait(&o->operai[riparazione],&o->m);
    }

    //o->operaio_occupato[riparazione] = true;
    printf("[CLINETE %d]\tL'OPERAIO PER LA MIA RIPARAZIONE %s e' LIBERO..\n",*pi);

    pthread_mutex_unlock(&o->m);
}

void attendi_riparazione (struct officina_t *o, int *pi, int riparazione){
    pthread_mutex_lock(&o->m);

    while (o->operaio_occupato[riparazione]){
        printf("[CLINETE %d]\tASPETTO FINE RIPARAZIONE\n",*pi);
        pthread_cond_wait(&o->ok_rip,&o->m);
    }

    printf("[CLINETE %d]\t\t\tRIPARAZIONE COMPLETATA\n",*pi);

    pthread_mutex_unlock(&o->m);
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

    for(int t =1;t<NTIMES;t++){
        printf("[CLINETE %d]\tSono arrivato in Officina\n",*pi);
        riparazione = rand() % NR;
        arrivo(&officina,pi,riparazione);
        printf("[CLINETE %d]\t\t\tIN RIPARAZIONE\n",*pi);
        attendi_riparazione(&officina,pi,riparazione);

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
    int NUM_CLIENTI, NUM_OPERAI;
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



