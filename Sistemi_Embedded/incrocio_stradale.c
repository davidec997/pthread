// devo farlo senza ciclo.. e fare che una macchina che e' arrivata dopo ad un incrocio, non puo passare se quella che la precede non ha attraversato
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
#define N 10 // dimensione coda; max macchine ammesse nella coda

typedef enum {false, true} Boolean;
char *colori [3] = {"ROSSO", "GIALLO", "VERDE"};

struct incrocio_t {
    sem_t m,incrocio_occupato;
    sem_t semaforo [4];
    //int direzione_occupata;
    int stato[4];
    int code_circolari[4][N];
    int head [4], tail[4];
    int coda_macchine[4];
    sem_t coda_piena, coda_vuota; // servono?
}incrocio;



void init_incrocio(struct incrocio_t *s){
    sem_init(&s->m,0,1);
    sem_init(&s->incrocio_occupato,0,4);        //devo controllare la direzione delle 2 auto    sem_init(&s->incrocio_occupato,0,4);        //devo controllare la direzione delle 2 auto
    sem_init(&s->coda_piena,0,0);
    sem_init(&s->coda_vuota,0,N);

    for (int i = 0; i < 4; i++) {
        sem_init(&s->semaforo[i], 0, 0);
        s->coda_macchine[i] = 0;
        s->stato[i] = -1;
        for (int j = 0; j < N; ++j) s->code_circolari[i][j] = -1;
        s->head[i] = s->tail[i] = 0;
    }

    //s->direzione_occupata = 0;

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
    // da riscrivere
    for (int i = 0; i < 4; i++) {
        if (s->stato[i] == VERDE && s->coda_macchine[i] > 0) {
            for (int j = 0; j < s->coda_macchine[i]; ++j) {
                sem_post(&s->semaforo[i]);
                s->coda_macchine[i] --;
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

Boolean sono_il_primo (struct incrocio_t *s,int sem_arrivo, int *pi){
    //devo controllare  che non ci sia nessuna auto che mi precede;se cosi fosse dovrei bloccarmi e svegliare questa

    if (s->code_circolari[sem_arrivo][s->head[sem_arrivo]] == *pi){
        //si sono il primo
        //printf("Macchina %d e' la prima in coda nel semafo %d e puo attraversare\n",*pi,sem_arrivo);
        return true;
    } else{
        //printf("Macchina %d NON e' la prima in coda nel semafo %d \n",*pi,sem_arrivo);
        return false;
    }
}

void controlla_attraversamento (struct incrocio_t *s,int sem_arrivo, int *pi){
    //determino se posso attraversare l'incrocio
    //posso attraversare se il mio semaforo e' verde e SOLO SE SONO IL PRIMO (HEAD) tra le macchine in coda al mio semaforo
    if (s->stato[sem_arrivo] == VERDE){
        if (sono_il_primo(s,sem_arrivo,*pi)){
            //posso attraversare
            sem_post(&s->semaforo[sem_arrivo]); //post previa
            printf("MACCHINA %d HA IL SEM VERDE ED E' LA PRIMA NELLA CODA.. ATTRAVERSA\n");
        } else  {
            printf("MACCHINA %d HA IL SEM VERDE MA NON E' LA PRIMA NELLA CODA.. NON PUO ATTRAVERSARE\n");
           // s->coda_macchine[sem_arrivo] ++;
        }
    }

    printf("-------------------------------------------\n");
    printf("Macchine bloccate sui semafori: ");
    for (int i = 0; i<4; i++) printf("%d\t",s->coda_macchine[i]);
    printf("\n");

    sem_post(&s->m);
    sem_wait(&s->semaforo[sem_arrivo]);
}



void termina_attraversamento (struct incrocio_t *s, int sem_arrivo,int *pi) {

    //aggiorna tutte le strutture
    //decrementa il num di macchine in coda
    //sleep(1);
    sem_wait(&s->m);
    //tolgo dalla head la macchina che ha appena attraversato
    s->coda_macchine[sem_arrivo] --;    //num macchine in coda al mio semaforo --
    s->code_circolari[sem_arrivo][s->head[sem_arrivo]] = -1; // setto a -1  cosi' in caso volessi fare un controllo in piu in fase di inserimento di una nuova macchina potrei verificare che ci sia un -1
    s->head[sem_arrivo] = (s->head[sem_arrivo] + 1) % N;  // incremento la head


    sem_post(&s->m);
    /*int index = (s->head[sem_arrivo] -1 ) % N; // calcolo a parte l indice della prossima macchina; cioe head -1 % N
    int prossima_macchina = s->code_circolari[sem_arrivo][index];
    s->code_circolari[sem_arrivo][s->head[sem_arrivo]] = prossima_macchina;*/

    //sem_post(&s->incrocio_occupato);

}

void arrivo_al_semaoro(struct incrocio_t *s,int sem_arrivo,int *pi){
    //registro che sono arrivato al semaforo
    //inserisco il mio id in coda
    //controllo se c'e' posto nella coda

    //MUTEX
    sem_wait(&s->m);
    if (s->coda_macchine[sem_arrivo] < N){
        //nella mia coda ci sono meno di N macchine accodate... posso accodarmi
        s->code_circolari[sem_arrivo][s->tail[sem_arrivo]] = *pi; // ho messo il mio id nella coda circolare
        s->head[sem_arrivo] = (s->tail[sem_arrivo] + 1 ) % N;
    } else {
        //non c' e' posto in coda... me ne vado
        sem_post(&s->m);
        pthread_exit((int *) -1);
    }
    //non rilascio il mutex
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
    sleep((rand()%5 + 1));


    int sem_arrivo = rand() % 4;  //scelgo in modo random su quale semaforo arriva la macchina. 0 e 1 rappresentano la strada mentre 2 e 3 l'altra

    //arrivo al semaforo
    printf("Sono l'Auto %d e sono arrivata sul semaforo \t%d\n",*pi,sem_arrivo);
    arrivo_al_semaoro(&incrocio,sem_arrivo,*pi);

    //controllo attraversamento
    controlla_attraversamento(&incrocio,sem_arrivo,*pi);
    //attraversa....
    printf("Sono l'Auto %d e sto attraversando l'incrocio \t%d\n",*pi,sem_arrivo);
    //termina attraversamento
    termina_attraversamento(&incrocio,sem_arrivo,*pi);

    printf("Sono l'Auto %d e ho liberato l'incrocio %d : sono passato [%d] volte\n",*pi,sem_arrivo,n_attraversamenti);
    printf("-------------------------------------------\n");
    //sleep(2);

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



