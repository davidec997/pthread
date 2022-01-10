//
// Created by davide on 27/12/21.
//
//NON VA


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

/*
Risoluzione del punto 1, con mutex e variabili condition o con semafori.

Il punto 2 è lasciato per esercizio :-)

Utilizzare l'apposita #define per scegliere l'implementazione...

(nell'implementazione con i mutex NON si è tenuto conto del problema della cancellazione dei threads).
*/

//#define USA_MUTEX
#define USA_SEM

/*
la struct R non è fornita dal testo; si sa solo che c'è e che va
utilizzata garantendo una certa concorrenza tra le varie attivazioni
delle funzioni ProcA, ProcB e Reset.

Per questo motivo, occorre fornire solo i tre processi PA, PB e PR.
Le procedure ProcA, ProcB e Reset in questa implementazione di esempio
non prendono parametri, in realtà potrebbero avere un puntatore ad una
struttura di tipo R.
*/

/*
   Nota bene:
   c_Reset vale sempre o 0 o 1!!!
*/



/* ------------------------------------------------------------------------ */

/* risoluzione con mutex e variabili condition */

#ifdef USA_MUTEX

pthread_mutex_t m;
pthread_cond_t priv_AB;
pthread_cond_t priv_Reset;
int c_AB, c_Reset;          // conta le istanze in esecuzione
int b_AB, b_Reset;          // conta il numero dei bloccati

void myInit(void)
{
  pthread_mutexattr_t m_attr;
  pthread_condattr_t c_attr;

  pthread_mutexattr_init(&m_attr);
  pthread_condattr_init(&c_attr);

  pthread_mutex_init(&m, &m_attr);
  pthread_cond_init(&priv_AB, &c_attr);
  pthread_cond_init(&priv_Reset, &c_attr);

  pthread_condattr_destroy(&c_attr);
  pthread_mutexattr_destroy(&m_attr);

  c_AB = c_Reset = b_AB = b_Reset = 0;
}

void StartProcA(void)
{
  pthread_mutex_lock(&m);
  while (c_Reset || b_Reset) {
    b_AB++;
    pthread_cond_wait(&priv_AB, &m);
    b_AB--;
  }
  c_AB++;
  pthread_mutex_unlock(&m);
}

void EndProcA(void)
{
  pthread_mutex_lock(&m);

  c_AB--;
  if (b_Reset && !c_AB)
    pthread_cond_signal(&priv_Reset);

  pthread_mutex_unlock(&m);
}


// le procedure di B si comportano in modo identico a quelle di A
void StartProcB(void)
{
  StartProcA();
}

void EndProcB(void)
{
  EndProcA();
}

void StartReset(void)
{
  pthread_mutex_lock(&m);
  while (c_AB) {
    b_Reset++;
    pthread_cond_wait(&priv_Reset, &m);
    b_Reset--;
  }
  c_Reset++;
  pthread_mutex_unlock(&m);
}

void EndReset(void)
{
  pthread_mutex_lock(&m);

  c_Reset--;
  if (b_AB)
    pthread_cond_broadcast(&priv_AB);

  pthread_mutex_unlock(&m);
}

#endif

/* ------------------------------------------------------------------------ */

/* risoluzione con semafori */

#ifdef USA_SEM

#define NESSUNO -1
#define S1      10
#define S2      20
#define F1      1
#define F2      2
#define F3      3
#define F4      4
#define F5      5
#define F6      6


sem_t m;
sem_t s_f1,s_f2,s_f3,s_f4,s_f5,s_f6;
int f1,f2,f3,f4,f5,f6;                       // conta le istanze in esecuzione
int f1_b,f2_b,f3_b,f4_b,f5_b,f6_b;          // conta il numero dei bloccati
int stato = NESSUNO;
int precedente;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&s_f1,0,0);
    sem_init(&s_f2,0,0);
    sem_init(&s_f3,0,0);
    sem_init(&s_f4,0,0);
    sem_init(&s_f5,0,0);
    sem_init(&s_f6,0,0);


    f1=f2=f3=f4=f5=f6= 0;
    f1_b=f2_b=f3_b=f4_b=f5_b=f6_b=0;
}

/*
 *  F1 | F2 --> F3 (possono eseguire + F3 cont) --> F4
 *  attivo tutte le istanze di f3 che arrivano durante f1 o f2.. non eseguo quelle che arrivano durante l esec di f3... senno f4 starva
 *
 *  F5 --> F6
 *  S1 : F1 | F2 -> F3 -> F4
 *  S2 : F5 -> F6
 *      S1 | S2 mutuamente esclusivo
 *
 * */

void StartF1(void)
{
    sem_wait(&m);
    if (stato == NESSUNO){
        stato = S1;
        f1 ++;
        precedente = F1;
        sem_post(&s_f1);
    } else
        f1_b ++;
    printf("\t\tF1 BLOCCATI %d\n",f1_b);

    sem_post(&m);
    sem_wait(&s_f1);
}

void EndF1(){
    sem_wait(&m);
    int n = f3_b;
    f1 --;
    if (f3_b > 0){
        while (n > 0){
            sem_post(&s_f3);
            precedente = F3;
            f3++;
            f3_b --;
            n--;
        }
    }
    sem_post(&m);
}

void StartF2(){
    sem_wait(&m);
    if (stato == NESSUNO){
        stato = S1;
        f2 ++;
        precedente = F2;
        sem_post(&s_f2);
    } else
        f2_b ++;
    printf("\t\tF2 BLOCCATI %d\n",f2_b);


    sem_post(&m);
    sem_wait(&s_f2);
}
void EndF2(){
    sem_wait(&m);
    int n = f3_b;
    f2 --;
    if (f3_b > 0){
        while (n > 0){
            sem_post(&s_f3);
            precedente = F3;
            f3++;
            f3_b --;
            n--;
        }
    }
    sem_post(&m);
}

void StartF3(){
    sem_wait(&m);
    if (stato == S1 && (precedente == F1 || precedente == F2 || precedente == F3)){
        precedente = F3;
        f3 ++;
        sem_post(&s_f3);
    } else
        f3_b ++;
        printf("\t\tF3 BLOCCATI %d\n",f3_b);


    sem_post(&m);
    sem_wait(&f3);

}

void EndF3(){
    sem_wait(&m);
    f3 --;
    int n = f3_b;
    // devo controllare se son l'ultimo dei F3
    if (f3 == 0 && f4_b > 0){
        // sveglio F4
        precedente = F4;
        f4++;
        f4_b--;
        sem_post(&s_f4);
    } else {
        while( n > 0){
            sem_post(&s_f3);
            f3_b --;
            f3 ++;
            precedente = F3;
            n--;
        }
    }
    sem_post(&m);
}

void StartF4(){
    sem_wait(&m);
    if (stato == S1 && precedente == F3 && f3 == 0){
        precedente = F4;
        f4 ++;
        sem_post(&s_f4);
    } else
        f4_b ++;
        printf("\t\tF4 BLOCCATI %d\n",f4_b);


    sem_post(&m);
    sem_wait(&f4);
}
void EndF4(){
    sem_wait(&m);
    f4 --;
    int n = f4_b;
    if (f4 == 0){
       //metto in seq nessuno
       stato = NESSUNO;
       if (f5_b > 0){
           stato = S2;
           sem_post(&s_f5);
           f5 ++;
           f5_b--;
           precedente = F5;
       } else if (f1_b > 0){
           f1_b --;
           stato = S1;
           f1 ++;
           sem_post(&s_f1);
           precedente = F1;
       } else if (f2_b){
           f2_b --;
           f2 ++;
           stato = S1;
           precedente = F2;
           sem_post(&s_f2);
       }
    } else {
        while( n > 0){
            sem_post(&s_f4);
            n--;
            precedente = F4;
            f4_b --;
            f4 ++;
        }
    }
    sem_post(&m);
}

void StartF5(){
    sem_wait(&m);
    if (stato = NESSUNO){
        stato = S2;
        f5 ++;
        precedente = F5;
        sem_post(&s_f5);
    } else
        f5_b ++;

    sem_post(&m);
    sem_wait(&s_f5);
}
void EndF5(){
    sem_wait(&m);
    f5 --;
    if (f6_b > 0){
        f6 ++;
        precedente = F6;
        f6_b --;
        sem_post(&s_f6);
    }
    sem_post(&m);
}

void StartF6(){
    sem_wait(&m);
    if (stato == S2 && f5 == 0 && precedente == F5){
        f6 ++;
        precedente = F6;
        sem_post(&s_f6);
    } else
        f6_b ++;
    sem_post(&m);
    sem_wait(&s_f6);
}
void EndF6(){
    sem_wait(&m);
    f6 --;
    stato = NESSUNO;
    if (f1_b){
        stato = S1;
        f1_b --;
        f1++;
        precedente = F1;
        sem_post(&s_f1);
    } else if (f2_b){
        stato = S1;
        f2 ++;
        f2_b --;
        precedente = F2;
        sem_post(&s_f2);
    } else if (f5_b){
        stato = S2;
        precedente = F5;
        f5_b --;
        f5 ++;
        sem_post(&s_f5);
    }
    sem_post(&m);
}

#endif

/* ------------------------------------------------------------------------ */

/* alla fine di ogni ciclo ogni thread aspetta un po'.
   Cosa succede se tolgo questa nanosleep?
   di fatto solo i thread di tipo B riescono ad entrare --> starvation!!!!
   (provare per credere)
*/
void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

/* ------------------------------------------------------------------------ */

/* le funzioni della risorsa R fittizia */

#define BUSY 1000000
#define CYCLE 50

void fun1 (  ){
    char *str = "F1";
    printf("[f1] ciao\n") ;
    StartF1();
    printf("%s\t",str);
    EndF1();
    pausetta();
}


void fun2 ( ){
    char *str = "F2";
    printf("[f2] ciao\n") ;

    StartF2();
    printf("%s\t",str);
    EndF2();
    pausetta();
}


void fun3 (  ){
    char *str = "F3";
    printf("[f3] ciao\n") ;
    StartF3();
    printf("%s\t",str);
    EndF3();
    pausetta();
}


void fun4 ( ){
    char *str = "F4";
    printf("[f4] ciao\n") ;
    StartF4();
    printf("%s\t",str);
    EndF4();
    pausetta();
}


void fun5 (  ){
    char *str = "F5";
    printf("[f5] ciao\n") ;
    StartF5();
    printf("%s\t",str);
    EndF5();
    pausetta();
}


void fun6 (  ){
    char *str = "F6";
    printf("[f6] ciao\n") ;
    StartF6();
    printf("%s\t",str);
    EndF6();
    pausetta();
}
/*

void myprint(char *s, int id)
{
    int i,j;
    fprintf(stderr,"[");
    for (j=0; j<CYCLE; j++) {
        fprintf(stderr,s,id);
        for (i=0; i<BUSY; i++);
    }
    fprintf(stderr,"]");
}
void ProcA(int id)
{
    printf("THREAD A CON ID %lu\n",id);
    myprint("%d",id);
}

void ProcB(int id)
{
    printf("THREAD B CON ID %lu\n",id);
    myprint("%d",id);
}

void Reset(int id)
{
    printf("THREAD R CON ID %lu\n",id);
    myprint("%d",id);
}


void *PA(void *arg)
{
    int id = (int) arg;
    printf(" CREATO THREAD A CON ID %lu\n",id);

    for (;;) {
        fprintf(stderr,"A");
        StartProcA();
        ProcA(id);
        EndProcA();
        fprintf(stderr,"a");
        sleep(1);
    }
    return 0;
}

void *PB(void *arg)
{
    int id = (int)arg;
    printf(" CREATO THREAD B CON ID %lu\n",id);

    for (;;) {
        fprintf(stderr,"B");
        StartProcB();
        ProcB(id);
        EndProcB();
        fprintf(stderr,"b");

        sleep(1);
    }
    return 0;
}

void *PR(void *arg)
{
    int id = (int)id;
    printf("CREATO THREAD R CON ID %lu\n",id);

    for (;;) {
        fprintf(stderr,"R");
        StartReset();
        Reset(id);
        EndReset();
        fprintf(stderr,"r");

    }
    return 0;
}
*/

void *client ( void * arg) {

    fun2();
    //fun4();
    //fun2();
    //sleep(1);
    fun3();
   // sleep(1);
    fun1();
    fun3();
   // sleep(1);

    fun4();
    fun1();
    fun2();
    fun3();
    /*int funzione;
    funzione = rand() % 6;
    switch (funzione) {
        case 0:
            fun1();
            break;
        case 1:
            fun2();
            break;
        case 2:
            fun3();
            break;
        case 3:
            fun4();
            break;
        case 4:
            fun5();
            break;
        case 5 :
            fun6();
            break;
    }*/

}

/* ------------------------------------------------------------------------ */
int main (int argc, char **argv)
{
    pthread_t *thread;
    pthread_t th;
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

    myInit();
    srand(555);
    pthread_create(&th, NULL, client, NULL);
   /* pthread_create(&th, NULL, client, NULL);
    pthread_create(&th, NULL, client, NULL);
    pthread_create(&th, NULL, client, NULL);
    pthread_create(&th, NULL, client, NULL);
    pthread_create(&th, NULL, client, NULL);
    pthread_create(&th, NULL, client, NULL);
*/

    pthread_join(th, (void**) & p);
    /*pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);
    pthread_join(th, (void**) & p);*/

    /*for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, client, NULL) != 0)
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
        // attendiamo la terminazione di tutti i thread generati
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d <- tot posti occupati\n", i, ris);
    }*/

    sleep(30);
    pthread_mutex_destroy(&m);
    exit(0);
}


