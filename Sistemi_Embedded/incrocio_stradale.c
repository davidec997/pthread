
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define COLORS 3
#define NTIMES 30
#define ROSSO 0
#define GIALLO 1
#define VERDE 2
#define LAMPEGGIANTE 3

typedef enum {false, true} Boolean;
char *colori [3] = {"ROSSO", "GIALLO", "VERDE"};
struct incrocio_t {
    sem_t m,incrocio_occupato;
    sem_t semaforo [4];
    int direzione_occupata;
    int macchine_blocc[4],stato[4];
}incrocio;



void init_incrocio(struct incrocio_t *s){
    sem_init(&s->m,0,1);
    sem_init(&s->incrocio_occupato,0,4);        //devo controllare la direzione delle 2 auto
    for (int i = 0; i < 4; i++) {
        sem_init(&s->semaforo[i],0,0);
        s->macchine_blocc[i] = 0;
        s->stato[i] = -1;
    }

    s->direzione_occupata = 0;

}

void rosso(struct incrocio_t *s,int direzione){
    sem_wait(&s->m);
    if (!direzione){
        s->stato[0] = ROSSO;
        s->stato[1] = ROSSO;
       // printf("\tHO SETTATO LO STATO DI SEM 0 E 1 A: %d\t\n",s->stato[0],s->stato[1]);
    }else{
        s->stato[2] = ROSSO;
        s->stato[3] = ROSSO;
        //printf("\tHO SETTATO LO STATO DI SEM 2 E 3 A: %d\t\n",s->stato[2],s->stato[3]);
    }
    sem_post(&s->m);
}

void verde(struct incrocio_t *s,int direzione){
    sem_wait(&s->m);
    if (!direzione){
        s->stato[0] = VERDE;
        s->stato[1] = VERDE;
        //printf("\tHO SETTATO LO STATO DI SEM 0 E 1 A: %d\t\n",s->stato[0],s->stato[1]);
    }else{
        s->stato[2] = VERDE;
        s->stato[3] = VERDE;
        //printf("\tHO SETTATO LO STATO DI SEM 2 E 3 A: %d\t\n",s->stato[2],s->stato[3]);

    }
    sem_post(&s->m);
}

void giallo(struct incrocio_t *s,int direzione){
    sem_wait(&s->m);
    if (!direzione){
        s->stato[0] = GIALLO;
        s->stato[1] = GIALLO;
       // printf("\tHO SETTATO LO STATO DI SEM 0 E 1 A: %d\t\n",s->stato[0],s->stato[1]);
    }else{
        s->stato[2] = GIALLO;
        s->stato[3] = GIALLO;
       // printf("\tHO SETTATO LO STATO DI SEM 2 E 3 A: %d\t\n",s->stato[2],s->stato[3]);
    }
    sem_post(&s->m);
}
void sveglia_bloccati(struct incrocio_t *s){
    for (int i = 0; i < 4; i++) {
        if (s->stato[i] == VERDE && s->macchine_blocc[i] > 0) {
            for (int j = 0; j < s->macchine_blocc[i]; ++j) {
                sem_post(&s->semaforo[i]);
                s->macchine_blocc[i] --;
            }
        }
    }
}

void *timer (void *arg){
    Boolean direzione = false; //0
    while(1){
        sleep(1);
        rosso(&incrocio,direzione);
        printf("SEMAFORO [%d] E [%d] ROSSO\n",0,1);
        sveglia_bloccati(&incrocio);
        verde(&incrocio,!direzione);
        printf("SEMAFORO [%d] E [%d] VERDE\n",2,3);
        sveglia_bloccati(&incrocio);
        sleep(3);
        giallo(&incrocio,!direzione);
        printf("SEMAFORO [%d] E [%d] GIALLO\n",2,3);
        sveglia_bloccati(&incrocio);
        sleep(1);
        rosso(&incrocio,!direzione);
        printf("SEMAFORO [%d] E [%d] ROSSO\n",2,3);
        sveglia_bloccati(&incrocio);
        verde(&incrocio,direzione);
        printf("SEMAFORO [%d] E [%d] VERDE\n",0,1);
        sveglia_bloccati(&incrocio);
        sleep(3);
        giallo(&incrocio,direzione);
        printf("SEMAFORO [%d] E [%d] GIALLO\n",0,1);
        sveglia_bloccati(&incrocio);

    }
}

void controlla_attraversamento (struct incrocio_t *s,int sem_arrivo){
    //determino se posso attraversare l'incrocio

    if (sem_arrivo == 0 || sem_arrivo == 1){
        //direzione "verticale"
        sem_wait(&s->m);
        if (s->stato[sem_arrivo] == VERDE){
            sem_post(&s->semaforo[sem_arrivo]); //post previa
        }else {
            printf("NON POSSO PASSARE PERCHE' Il SEMAFORO %d  E' %s\n", sem_arrivo, colori[s->stato[sem_arrivo]]);
            s->macchine_blocc[sem_arrivo] ++;
        }
        //se il sem e' giallo o rosso mi blocco
        sem_post(&s->m);
        sem_wait(&s->incrocio_occupato);
        sem_wait(&s->semaforo[sem_arrivo]);

    }else{
        sem_wait(&s->m);
        if (s->stato[sem_arrivo] == VERDE){
            sem_post(&s->semaforo[sem_arrivo]); //post previa
        }else {
            printf("NON POSSO PASSARE PERCHE' Il SEMAFORO %d  E' %s\n", sem_arrivo, colori[s->stato[sem_arrivo]]);
            s->macchine_blocc[sem_arrivo] ++;
        }

        sem_post(&s->m);
        sem_wait(&s->semaforo[sem_arrivo]);
        sem_wait(&s->incrocio_occupato);
    }


    printf("-------------------------------------------\n");
    printf("Macchine bloccate sui semafori: ");
    for (int i = 0; i<4; i++) printf("%d\t",s->macchine_blocc[i]);
    printf("\n");
}



void termina_attraversamento (struct incrocio_t *s, int sem_arrivo) {

    sleep(1);
    sem_post(&s->incrocio_occupato);

}

void *automobile(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    int n_attraversamenti=0;
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    //aspetto qualche secondo per far inizializzare correttamente i semafori e i loro stati
    sleep(6);

    for (int i=0; i< NTIMES; i++){
        int sem_arrivo = rand() % 4;  //scelgo in modo random su quale semaforo arriva la macchina. 0 e 1 rappresentano la strada mentre 2 e 3 l'altra
        sleep((rand()%5 + 1));
        //controlla attraversamento
        printf("Sono l'Auto %d e sono arrivato sul semaforo \t%d\n",*pi,sem_arrivo);
        controlla_attraversamento(&incrocio,sem_arrivo);
        //attraversa....
        printf("Sono l'Auto %d e sto attraversando l'incrocio \t%d\n",*pi,sem_arrivo);
        //termina attraversamento
        termina_attraversamento(&incrocio,sem_arrivo);
        n_attraversamenti ++;
        printf("Sono l'Auto %d e ho liberato l'incrocio %d : sono passato [%d] volte\n",*pi,sem_arrivo,n_attraversamenti);
        printf("-------------------------------------------\n");
        sleep(2);
    }


    *ptr = 0;
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

    init_incrocio(&incrocio);

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


    srand(555);

    taskids[0] = 0;
    pthread_create(&thread[i], NULL, timer, (void *) (&taskids[0]));
    sleep(2);

    for (i=1; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, automobile, (void *) (&taskids[i])) != 0)
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
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    exit(0);
}




