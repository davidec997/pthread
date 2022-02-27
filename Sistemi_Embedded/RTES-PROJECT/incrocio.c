#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//Colori semafori
#define ROSSO 0
#define GIALLO 1
#define VERDE 2

// DIM coda --> max macchine ammesse nella coda
#define N 20

// parametro per stampe di controllo
#define N_STAMPE 120

//timer semaforo
#define SEM_TIMER 5

typedef enum {false, true} Boolean;

//struttura incrocio
struct incrocio_t {
    pthread_cond_t stop[4];     // una cond var per ogni semaforo
    pthread_mutex_t mtx;        // mutex per accedere alle variabili condivise

    int stato[4];               // stato di ogni semaforo
    int code_circolari[4][N];   // ogni semaforo ha la sua coda circolare di N posti
    int head [4], tail[4];      // una head e una tail per ogni coda circolare
    int coda_macchine[4];       // array per la coda delle macchine, una per ogni semaforo

}incrocio;


void sveglia_bloccati(struct incrocio_t *s, Boolean direzione);         // prototipo funzione sveglia_bloccati


void init_incrocio(struct incrocio_t *s){
    // inizializzo la struct
    pthread_mutex_init(&s->mtx,NULL);                                   //inizializzo il mutex

    for (int i = 0; i < 4; i++) {
        pthread_cond_init(&s->stop[i],NULL);                            //inizializzo le condition variables

        s->coda_macchine[i] = 0;                                        //inizializzo a 0 in num di macchine in coda su ogni semaforo
        s->stato[i] = -1;                                               //inizializzo lo stato dei semafori ad uno stato non valido
        for (int j = 0; j < N; ++j) s->code_circolari[i][j] = -1;
        s->head[i] = s->tail[i] = 0;                                    // setto tutti gli elementi della coda a -1
    }
}

void rosso(struct incrocio_t *s,int direzione){
    //setta il rosso sui semafori della stessa direzione --> 0 e 1 oppure 2 e 3, in base al parametro direzione
    pthread_mutex_lock(&s->mtx);
    if (!direzione){                                                    // direzione 0 --> orizzontale --> semafori 0 e 1
        s->stato[0] = ROSSO;
        s->stato[1] = ROSSO;
        printf("\t\t\t\t\t\tSEMAFORO [0] E [1] ROSSO\n");
    }else{                                                              // direzione 1 --> verticale --> semafori 2 e 3
        s->stato[2] = ROSSO;
        s->stato[3] = ROSSO;
        printf("\t\t\t\t\t\tSEMAFORO [2] E [3] ROSSO\n");
    }
    pthread_mutex_unlock(&s->mtx);
}

void verde(struct incrocio_t *s,int direzione){
    pthread_mutex_lock(&s->mtx);
    if (!direzione){
        s->stato[0] = VERDE;
        s->stato[1] = VERDE;
        printf("\t\t\t\t\t\tSEMAFORO [0] E [1] VERDE\n");
    }else{
        s->stato[2] = VERDE;
        s->stato[3] = VERDE;
        printf("\t\t\t\t\t\tSEMAFORO [2] E [3] VERDE\n");
    }
    pthread_mutex_unlock(&s->mtx);
}

void giallo(struct incrocio_t *s,int direzione){
    pthread_mutex_lock(&s->mtx);
    if (!direzione){
        s->stato[0] = GIALLO;
        s->stato[1] = GIALLO;
        printf("\t\t\t\t\t\tSEMAFORO [0] E [1] GIALLO\n");
    }else{
        s->stato[2] = GIALLO;
        s->stato[3] = GIALLO;
        printf("\t\t\t\t\t\tSEMAFORO [2] E [3] GIALLO\n");
    }
    pthread_mutex_unlock(&s->mtx);
}

void sveglia_bloccati(struct incrocio_t *s, Boolean direzione){
    //sveglio le auto bloccate in base al parametro direzione
    if (!direzione){                                            //direzione 0 --> sveglio auto del sem 0 e 1
        pthread_cond_broadcast(&s->stop[0]);
        pthread_cond_broadcast(&s->stop[1]);
    } else {                                                    //direzione 1 --> sveglio auto del sem 2 e 3
        pthread_cond_broadcast(&s->stop[2]);
        pthread_cond_broadcast(&s->stop[3]);
    }
}


void *timer (void *arg){
    // thread timer che gestisce i 4 semafori
    Boolean direzione = false; //0 --> direzione verticale         1--> direzione orizzontale
    while(1){
        rosso(&incrocio,direzione);                                 //rosso per direzione 1
        verde(&incrocio,!direzione);                                //verde per direzione 0
        sveglia_bloccati(&incrocio,!direzione);                     //sveglio i thread bloccati sulla direzione dove e' appena scattato il verde
        sleep(SEM_TIMER);

        giallo(&incrocio,!direzione);                               //giallo per direzione 0
        sleep(1);                                                   //il giallo dura solo 1 secondo

        rosso(&incrocio,!direzione);                                //rosso per direzione 0
        verde(&incrocio,direzione);                                 //verde per direzione 1
        sveglia_bloccati(&incrocio,direzione);                      //sveglio i bloccati su direzione 1
        sleep(SEM_TIMER);

        giallo(&incrocio,direzione);                                //giallo per direzine 1
        sleep(1);
    }
}

Boolean sono_il_primo (struct incrocio_t *s,int sem_arrivo, int *pi){
    //devo controllare che non ci sia nessuna auto che mi precede
    if (s->code_circolari[sem_arrivo][s->tail[sem_arrivo]] == *pi){
        //si sono il primo
        return true;
    } else{
        // non sono il primo
        return false;
    }
}

void controlla_attraversamento (struct incrocio_t *s,int sem_arrivo, int *pi){
    //posso attraversare se il mio semaforo e' verde e SOLO SE SONO LA PRIMA (HEAD) tra le macchine in coda al mio semaforo
    pthread_mutex_lock(&s->mtx);
    //se il mio semaforo non e' verde o non sono il primo... mi blocco
    while(!((s->stato[sem_arrivo] == VERDE)  && (sono_il_primo(s,sem_arrivo,pi)))){
        printf("[GESTORE]\t\t\tMACCHINA %d NON HA IL SEMAFORO\t[%d] VERDE O NON E' LA PRIMA NELLA CODA.. NON PUO' ATTRAVERSARE\n",*pi,sem_arrivo);
        pthread_cond_wait(&s->stop[sem_arrivo],&s->mtx);
    }
    printf("[GESTORE]\t\t\tMACCHINA %d HA IL SEMAFORO\t[%d] VERDE ED E' LA PRIMA NELLA CODA.. PUO' ATTRAVERSARE\n",*pi,sem_arrivo);
    pthread_mutex_unlock(&s->mtx);
}


void termina_attraversamento (struct incrocio_t *s, int sem_arrivo,int *pi) {
    //aggiorno tutte le strutture
    //decremento il num di macchine in coda
    pthread_mutex_lock(&s->mtx);

    //tolgo dalla coda la macchina che ha appena attraversato
    s->coda_macchine[sem_arrivo] --;                            //decremento il num macchine in coda al mio semaforo
    s->tail[sem_arrivo] = (s->tail[sem_arrivo] + 1) % N;        //incremento la tail

    //stampe di controllo ---------------------------------------------------
    printf("\n");
    for(int t=0; t<N_STAMPE; t++) printf("*");
    printf("\n[GESTORE]\t\t\tMacchine bloccate sui semafori: ");
    for (int i = 0; i<4; i++) printf("%d\t",s->coda_macchine[i]);
    printf("\n");
    for(int t=0; t<N_STAMPE; t++) printf("*");

    printf("\n");
    //stampe di controllo ---------------------------------------------------

    pthread_mutex_unlock(&s->mtx);
    pthread_cond_broadcast(&s->stop[sem_arrivo]);           //sveglio solo le auto del mio semaforo
}

void arrivo_al_semaforo(struct incrocio_t *s,int sem_arrivo,int *pi){
    //registro che sono arrivato al semaforo
    //controllo se c'e' posto nella coda, altrimenti esco
    //inserisco il mio id in coda
    int *ptr;
    ptr = (int *) malloc( sizeof(int));

    pthread_mutex_lock(&s->mtx);

    if (s->coda_macchine[sem_arrivo] < N){
        //nella mia coda ci sono meno di N macchine accodate... posso accodarmi
        s->code_circolari[sem_arrivo][s->head[sem_arrivo]] = *pi;               // metto il mio id nella coda circolare
        printf("[GESTORE]\t\t\t[Ho messo la MACCHINA\t%d in posizione\t[%d] nella coda\t%d\n",*pi,s->head[sem_arrivo],sem_arrivo);
        s->head[sem_arrivo] = (s->head[sem_arrivo] + 1 ) % N;                   // aggiorno la head
        s->coda_macchine[sem_arrivo] ++;                                        // incremento il numero di macchine nella coda circolare
    } else {
        //non c' e' posto in coda... inversione a u e vado via
        printf("\t[AUTO %d]\t\t\t NON E' RIUSCITA AD ACCODARSI... CODA [%d] PIENA... ESCO\n",*pi,sem_arrivo);       // se non c'e' posto nella coda..
        pthread_mutex_unlock(&s->mtx);                                          //rilascio il mutex...
        *ptr = -1;                                                              //valore di ritorno -1 per segnalare di non essersi accodata..
        pthread_exit((void *) ptr);                                             //..ed esco
    }

    printf("[AUTO %d]\t\t\tSono arrivata sul semaforo \t%d\n",*pi,sem_arrivo);
    pthread_mutex_unlock(&s->mtx);
}

void *automobile(void *id){
    int *pi = (int *)id;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    int n_attraversamenti=0;
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    //aspetto qualche secondo per far inizializzare correttamente i semafori e i loro stati
    //aspetto anche per distribuire nel tempo l'arrivo delle auto
    sleep((rand()%15 + 1));

    int sem_arrivo = rand() % 4;  //scelgo in modo random su quale semaforo arriva la macchina

    //arrivo al semaforo
    arrivo_al_semaforo(&incrocio,sem_arrivo,pi);

    //controllo attraversamento
    controlla_attraversamento(&incrocio,sem_arrivo,pi);

    //attraversa....
    printf("[AUTO %d]\t\t\tSto attraversando l'incrocio dal semaforo \t%d\n",*pi,sem_arrivo);
    sleep(1);

    //termina attraversamento
    termina_attraversamento(&incrocio,sem_arrivo,pi);
    n_attraversamenti ++;           // segno che ho attraversato

    // stampe di controllo -----------------------------------------------------------------------------------------------
    pthread_mutex_lock(&incrocio.mtx);      //mutex solo per le stampe
    printf("[AUTO %d]\t\t\tho liberato l'incrocio %d : sono passato %d volta\n",*pi,sem_arrivo,n_attraversamenti);
    pthread_mutex_unlock(&incrocio.mtx);
    // stampe di controllo -----------------------------------------------------------------------------------------------


    *ptr = n_attraversamenti;
    //pthread_exit((void *) ptr);
}



int main (int argc, char **argv){
    pthread_t *thread;
    pthread_attr_t a;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS;
    char error[250];

    // Controllo sul numero di parametri
    if (argc != 2 ){        // Deve essere passato esattamente un parametro
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    //Calcoliamo il numero passato che sara' il numero di Pthread da creare
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0) {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
    }

    // inizializziamo la struttura
    init_incrocio(&incrocio);
    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);


    //allocazione memoria per i Pthreads
    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL){
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }

    //allocazione memoria per taskids
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL){
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    //inizializziamo il seme della rand()
    srand(time(NULL));


    //creazione dei threads
    for (i=1; i < NUM_THREADS; i++){
        taskids[i] = i;
        if (i == NUM_THREADS - 1)   //l'ultimo thread e' il timer
            if (pthread_create(&thread[i], &a, timer, (void *) (&taskids[i])) != 0){
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(5);
            }

        if (pthread_create(&thread[i], &a, automobile, (void *) (&taskids[i])) != 0){
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
    }

     // aspetto la terminazione solo dei thread auto
    /* for (i=0; i < NUM_THREADS; i++){
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d <-- numero di attraversamenti\n", i, ris);
    }  */

    sleep (30);
    return 0;
}




