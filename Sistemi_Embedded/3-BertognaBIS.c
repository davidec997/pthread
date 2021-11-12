//3 bertogna
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define N 5
#define DELAY 20000000
typedef int T;
typedef struct busta_t{
    T messaggio;
    int priorita;
}busta;

int valGlobale=1;

typedef enum {false,true} Boolean;

struct gestore_t {
    int buste_disp, mb_liberi;
    int mit_b_buste,mb_mit,mb_ric; //mitt bloccato xk non ci sono buste; mitt blocc se mailbox piena; ric blocc xk mailbox vuota
    busta b;
    busta mailbox[N];
    sem_t mb_vuota_sem,mb_piena_sem, pool_buste_sem, m;
    T msg;
    int head,tail;
}gestore;

//ricevente
/* for(int i=0;i<3;i++){
    for(int l=tail%N;l<=head%N;l++){
        if(mailbox[l%N]!= -1 && mailbox[l%N]->busta.priorita == i){
            //ho trovato il primo a priorita piu alta;
        }

}*/


//mittente prio 1


void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}
void myInit(struct gestore_t *g)
{
    g->mb_liberi = N;
    g->buste_disp = 4;
    g->msg = -1;
    g->b.messaggio =-1;
    g->b.priorita=0;  // -1
    for(int i=0;i<N;i++) g->mailbox[i]= g->b;
    sem_init(&g->m,0,1);
    sem_init(&g->pool_buste_sem, 0, N);
    sem_init(&g->mb_vuota_sem, 0, N);           // N posti liberi
    sem_init(&g->mb_piena_sem, 0, 0);      // 0 posti occupati
    g->head= g->tail = 0;

}

struct busta_t richiedi_busta(struct gestore_t *g, int priorita){
    sem_wait(&g->m);
    busta b1;
    if (g->buste_disp > 0){
        g->buste_disp --;
        b1.priorita = priorita;
        sem_post(&g->pool_buste_sem); //post previa per send
    } else {
        g->mit_b_buste ++;
    }
    sem_post(&g->m);
    sem_wait(&g->pool_buste_sem);  // receive mi deve fare post senno dedlk
    return b1;
}

void rimetti_busta(struct gestore_t *g){
    sem_wait(&g->m);
    g->buste_disp ++;
    sem_post(&g->pool_buste_sem);
    sem_post(&g->m);
}
void *send(struct gestore_t *g) {
    while (1) {
        //controllo se mb piena
        sem_wait(&g->m);
        if (g->mb_liberi <= 0) {
            //mi blocco
            g->mb_mit++;
            sem_post(&g->m);
            sem_wait(&g->mb_vuota_sem); // receiver deve fare post; aspetto che ci sia un posto libero
        } else {
            sem_wait(&g->mb_vuota_sem); // consumo un posto libero dalla mb
            g->mb_liberi --;
            busta b = richiedi_busta(g, rand() % 3);
            //sem_wait(&g->m);
            b.messaggio = valGlobale += 1;
            //metto il msg nella mailbox in pos head
            g->mailbox[g->head % N] = b;
            g->head = (g->head + 1 ) % N;
            sem_post(&g->m);                        // VUOTA   N     PIENA  0
            sem_post(&g->mb_piena_sem); // segnalo ai receiver bloccati
        }
    }
}

void *receive(struct gestore_t *g)
{
    while(1) {
        busta b;
        int i;
        sem_wait(&g->m);
        if((N - g->mb_liberi) >0){
            sem_wait(&g->mb_piena_sem);
            g->mb_liberi --;
            /*for(int i= 0; i<3;i++){
                for(;;);
            }*/
            b  = g->mailbox[i];
        } else {

        }
        sem_wait(&g->mb_piena_sem);
        sem_wait(&g->m);
        //ciclo impossibile che mi dara l indice della casella nella mailbox da estrarre
        busta r = g->mailbox[g->tail];
        g->tail = (g->tail + 1) % N;
        printf("Ho estratto una busta a priorita %d con messaggio \t%d\n", r.priorita, r.messaggio);
        sem_post(&g->m);
        rimetti_busta(g);
    }

}



int main(int argc, char *argv[]) {
    pthread_t *threadM;
    pthread_t *threadR;

    int *taskids;
    int i;
    int *p;
    int NUM_THREADS_R;
    int NUM_THREADS_M;
    char error[250];
    /* Controllo sul numero di parametri */
    if (argc != 3 ) /* Deve essere passato esattamente un parametro */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS_M = atoi(argv[1]);
    NUM_THREADS_R = atoi(argv[2]);

    if (NUM_THREADS_R <= 0 || NUM_THREADS_M <= 0)
    {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS_R);
        perror(error);
        exit(2);
    }

    myInit(&gestore);

    threadM=(pthread_t *) malloc((NUM_THREADS_M) * sizeof(pthread_t));
    threadR=(pthread_t *) malloc((NUM_THREADS_R) * sizeof(pthread_t));

    if (threadM == NULL || threadR == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskids = (int *) malloc((NUM_THREADS_R ) * sizeof(int));
    if (taskids == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    for (i=0; i < NUM_THREADS_M; i++)
    {
        //taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&threadM[i], NULL, send, (rand() % 3)) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread LETTORE %i-esimo con id=%lu\n", i, threadM[i]);
    }
    for (i=0; i < NUM_THREADS_R; i++)
    {
        taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&threadR[i], NULL, receive, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread SCRITTORE %i-esimo con id=%lu\n", i, threadS[i]);
    }

    sleep(10);
    for (i=0; i < NUM_THREADS_M; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(threadM[i], (void**) & p);
        ris= *p;
        printf("Pthread Lettore %d-esimo restituisce %d\n", i, ris);
    }
    for (i=0; i < NUM_THREADS_R; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(threadR[i], (void**) & p);
        ris= *p;
        printf("Pthread Scrittore %d-esimo restituisce %d\n", i, ris);
    }

    exit(0);
}

