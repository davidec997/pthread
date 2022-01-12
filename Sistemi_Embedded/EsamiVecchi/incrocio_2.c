// devo farlo senza ciclo.. e fare che una macchina che e' arrivata dopo ad un incrocio, non puo passare se quella che la precede non ha attraversato
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define ROSSO 0
#define GIALLO 1
#define VERDE 2
#define N 10 // dimensione coda; max macchine ammesse nella coda
#define MAX_DELAY 30
#define N_STAMPE 120
#define SEM_TIMER 5

typedef enum {false, true} Boolean;
char *colori [3] = {"ROSSO", "GIALLO", "VERDE"};

struct incrocio_t {
    sem_t code[4];
    pthread_mutex_t mtx;
    int stato[4];     //  X VERSIONE CON SEMAFORI REALI
    //int stato;
    int coda_macchine[4];
    int attraversando[4];


}incrocio;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void sveglia_bloccati(struct incrocio_t *s, int direzione);


void init_incrocio(struct incrocio_t *s){

    pthread_mutex_init(&s->mtx,NULL);

    for (int i = 0; i < 4; i++) {
        s->coda_macchine[i] = 0;
        s->attraversando[i] = 0;
        s->stato[i] = -1;
        sem_init(&s->code[i], 0,0);
    }
   // s->stato = -1;
}

void rosso(struct incrocio_t *s,int direzione){
    pthread_mutex_lock(&s->mtx);
    //s->stato = ROSSO;
    if (!direzione){
        s->stato[0] = ROSSO;
        s->stato[1] = ROSSO;
        // printf("\tHO SETTATO LO STATO DI SEM 0 E 1 A: %d\t\n",s->stato[0],s->stato[1]);
    }else{
        s->stato[2] = ROSSO;
        s->stato[3] = ROSSO;
        //printf("\tHO SETTATO LO STATO DI SEM 2 E 3 A: %d\t\n",s->stato[2],s->stato[3]);
    }
    pthread_mutex_unlock(&s->mtx);
   // sveglia_bloccati(s,direzione);

}

void verde(struct incrocio_t *s,int direzione){
    pthread_mutex_lock(&s->mtx);
    //s->stato = VERDE;

    if (!direzione){
        s->stato[0] = VERDE;
        s->stato[1] = VERDE;
        //printf("\tHO SETTATO LO STATO DI SEM 0 E 1 A: %d\t\n",s->stato[0],s->stato[1]);
    }else{
        s->stato[2] = VERDE;
        s->stato[3] = VERDE;
        //printf("\tHO SETTATO LO STATO DI SEM 2 E 3 A: %d\t\n",s->stato[2],s->stato[3]);
    }
    pthread_mutex_unlock(&s->mtx);
    sveglia_bloccati(s,direzione);

}

void giallo(struct incrocio_t *s,int direzione){
    pthread_mutex_lock(&s->mtx);
   // s->stato = GIALLO;
     if (!direzione){
         s->stato[0] = GIALLO;
         s->stato[1] = GIALLO;
         // printf("\tHO SETTATO LO STATO DI SEM 0 E 1 A: %d\t\n",s->stato[0],s->stato[1]);
     }else{
         s->stato[2] = GIALLO;
         s->stato[3] = GIALLO;
         // printf("\tHO SETTATO LO STATO DI SEM 2 E 3 A: %d\t\n",s->stato[2],s->stato[3]);
     }
    pthread_mutex_unlock(&s->mtx);
    //sveglia_bloccati(s,direzione);
}
void sveglia_bloccati(struct incrocio_t *s, int direzione){
    //sveglio tutti
    pthread_mutex_lock(&s->mtx);
    for (int i = 0; i < 4; i++) {
        if (s->coda_macchine[i]){
            s->attraversando[i]++;
            sem_post(&s->code[i]);
        }
    }

    pthread_mutex_unlock(&s->mtx);

    //for(int i =0; i<4; i++) pthread_cond_broadcast(&s->stop[i]);
}

void *timer (void *arg){
    Boolean direzione = false; //0 --> direzione verticale 1--> direzione orizzontale
    while(1){
        rosso(&incrocio,direzione);
        printf("[SEMAFORO]\t\tROSSO\n");
        sleep(SEM_TIMER);
        giallo(&incrocio,direzione);
        printf("[SEMAFORO]\t\tGIALLO\n");
        sleep(1);
        verde(&incrocio,direzione);
        printf("[SEMAFORO]\t\tVERDE\n");
        sleep(SEM_TIMER);

        /*rosso(&incrocio,direzione);
        printf("\t\t\t\t\t\tSEMAFORO [%d] E [%d] ROSSO\n",0,1);
        verde(&incrocio,!direzione);
        printf("\t\t\t\t\t\tSEMAFORO [%d] E [%d] VERDE\n",2,3);
        sveglia_bloccati(&incrocio,!direzione);        //quando scatta il verde sveglio i thread bloccati
        sleep(SEM_TIMER);
        giallo(&incrocio,!direzione);
        printf("\t\t\t\t\t\tSEMAFORO [%d] E [%d] GIALLO\n",2,3);
        sleep(1);
        rosso(&incrocio,!direzione);
        printf("\t\t\t\t\t\tSEMAFORO [%d] E [%d] ROSSO\n",2,3);
        verde(&incrocio,direzione);
        printf("\t\t\t\t\t\tSEMAFORO [%d] E [%d] VERDE\n",0,1);
        sveglia_bloccati(&incrocio,direzione);
        sleep(SEM_TIMER);
        giallo(&incrocio,direzione);
        printf("\t\t\t\t\t\tSEMAFORO [%d] E [%d] GIALLO\n",0,1);*/
    }
}

void controlla_attraversamento (struct incrocio_t *s,int sem_arrivo, int *pi){
    //posso attraversare se il mio semaforo e' verde e SOLO SE SONO IL PRIMO (HEAD) tra le macchine in coda al mio semaforo
    pthread_mutex_lock(&s->mtx);
    s->coda_macchine[sem_arrivo] ++;
    printf("[AUTO %d]\t\t\tSono arrivata sul semaforo \t%d\n",*pi,sem_arrivo);
    // controllos e' verde e se non ci sono auto prima di me
    if (s->stato == VERDE && s->coda_macchine[sem_arrivo] == 1){
        // attraverso
        s->attraversando[sem_arrivo] ++;
        sem_post(&s->code[sem_arrivo]);
        printf("[AUTO %d]\t\t\tposso attraversare sul semaforo \t%d\n",*pi,sem_arrivo);

    }
    pthread_mutex_unlock(&s->mtx);
    sem_wait(&s->code[sem_arrivo]);
}


void termina_attraversamento (struct incrocio_t *s, int sem_arrivo,int *pi) {
    pthread_mutex_lock(&s->mtx);
    s->coda_macchine[sem_arrivo] --;
    s->attraversando[sem_arrivo] --;

    if (s->stato == VERDE && s->coda_macchine[sem_arrivo] > 0){
        sem_post(&s->code[sem_arrivo]);
      //  s->coda_macchine[sem_arrivo] --;
        s->attraversando[sem_arrivo] ++;
    }

    pthread_mutex_unlock(&s->mtx);


}

void *automobile(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    int n_attraversamenti=0;
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    //aspetto qualche secondo per far inizializzare correttamente i semafori e i loro stati

    sleep((rand()%8 + 2));

    int sem_arrivo = rand() % 4;  //scelgo in modo random su quale semaforo arriva la macchina. 0 e 1 rappresentano la strada mentre 2 e 3 l'altra

    //arrivo al semaforo
    //arrivo_al_semaoro(&incrocio,sem_arrivo,pi);
    printf("[AUTO %d]\t\t\tSono arrivata sul semaforo \t%d\n",*pi,sem_arrivo);

    //controllo attraversamento
    controlla_attraversamento(&incrocio,sem_arrivo,pi);
    //attraversa....
    printf("[AUTO %d]\t\t\tSto attraversando l'incrocio dal semaforo \t%d\n",*pi,sem_arrivo);
    //sleep(1);
    pausetta();
    sleep(1);
    //termina attraversamento
    termina_attraversamento(&incrocio,sem_arrivo,pi);
    n_attraversamenti ++;

    pthread_mutex_lock(&incrocio.mtx);      //mutex solo per le stampe
    printf("[AUTO %d]\t\t\tho liberato l'incrocio %d : sono passato %d volta\n",*pi,sem_arrivo,n_attraversamenti);
    printf("\n");
    for(int t=0; t<N_STAMPE; t++) printf("*");
    printf("\n");
    pthread_mutex_unlock(&incrocio.mtx);
    *ptr = n_attraversamenti;
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





