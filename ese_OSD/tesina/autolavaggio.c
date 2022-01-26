
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define POSTI 5                                             //numero di auto che l'autolavaggio puo lavare contemporaneamente
#define DELAY 30                                            //max ritardo d'esecuzione per i thread

int auto_in_lavaggio, auto_bloccate;                        //contatori per auto in lavaggio e bloccate
int camper_in_lavaggio, camper_bloccati;                    //contatori per camper in lavaggio e bloccati
int posti_liberi;                                           //posti disponibili per le macchine
sem_t s_auto, s_camper,m;                                   //mutex, semaforo per le auto e per i camper

//inizializzazione struttura
void autolavaggio_init(void){
    //init semafori
    sem_init(&m,0,1);
    sem_init(&s_auto, 0, 0);
    sem_init(&s_auto, 0, 0);
    //inizializzo posti_liberi
    posti_liberi = POSTI;
    auto_in_lavaggio = auto_bloccate = camper_in_lavaggio = camper_bloccati = 0;
}

void inizioLavaggio(){
    sem_wait(&m);                                                       //mutex

    //controllo entrata per thread auto: non ci devono essere camper in lavaggio o bloccati e ci deve essere almeno un posto libero
    if(!camper_in_lavaggio && !camper_bloccati  && posti_liberi > 0){
        auto_in_lavaggio ++;                                                //incremento auto_in_lavaggio
        posti_liberi --;                                                    //decremento posti liberi
        sem_post(&s_auto);                                             // post previa sul semaforo delle auto
    }else{
        auto_bloccate ++;                                                   //altrimenti segno che c'e' un'auto bloccata in piu'
    }

    sem_post(&m);                                                       //rilascio il mutex...
    sem_wait(&s_auto);                                                  //wait su auto --> passante solo se e' stata eseguita la post previa
}


void fineLavaggio(){
    sem_wait(&m);

    auto_in_lavaggio --;                                                    //decremento le auto in lavaggio
    posti_liberi ++;                                                        //incremento posti liberi

    //stampa di controllo
    printf("\n*[SERVICE]*\t\t\t\tAUTO BLOCCATE %d\tPOSTI LIBERI %d\tCAMPER BLOCCATI %d\n",auto_bloccate,posti_liberi,camper_bloccati);

    //un'auto deve dare sempre precedenza a camper bloccati se ce ne sono
    if(!auto_in_lavaggio && camper_bloccati > 0 ){                          //se non ci sono piu' auto in lavaggio e c'e' almeno un camper bloccato... lo sveglio
        sem_post(&s_camper);                                           //post sul semaforo sei camper
        camper_bloccati --;                                                 //book keeping fatto dal processo svegliante
        camper_in_lavaggio ++;
    } else if (auto_bloccate > 0 && posti_liberi > 0 && camper_bloccati == 0){      //solo se non ci sono camper bloccati controllo se ci sono auto bloccate
        auto_in_lavaggio ++;
        posti_liberi --;
        auto_bloccate --;
        sem_post(&s_auto);
    }
    sem_post(&m);
}


void inizioLavaggioCamper(){
    sem_wait(&m);
    //un camper puo' entrare solo se non ci sono ne auto ne camper in lavaggio
    if(!camper_in_lavaggio && !auto_in_lavaggio){
        sem_post(&s_camper);                                            //post previa
        camper_in_lavaggio++;
    } else{
        camper_bloccati ++;
    }
    sem_post(&m);
    sem_wait(&s_camper);
}


void fineLavaggioCamper(){
    sem_wait(&m);
    camper_in_lavaggio --;

    //stampa di controllo
    printf("\n*[SERVICE]*\t\t\t\tAUTO BLOCCATE %d\tPOSTI LIBERI %d\tCAMPER BLOCCATI %d\n",auto_bloccate,posti_liberi,camper_bloccati);

    //anche i camper controllano prima se ci sono camper bloccati e solo in caso negativo cercano di svegliare auto bloccate
    if (camper_bloccati > 0){                                   //non c'e' bisogno di controllare che non ci siano altri camper in lavaggio --> solo un camper alla volta puo' essere lavato
        camper_bloccati--;
        camper_in_lavaggio ++;
        sem_post(&s_camper);
    } else if(auto_bloccate > 0){                               //se ci sono auto bloccate..
        while(auto_bloccate > 0 && posti_liberi > 0) {          //..cerco di svegliarne il piu' possibile  --> min (auto_bloccate, posti_liberi)
            auto_bloccate--;
            auto_in_lavaggio++;
            posti_liberi --;
            sem_post(&s_auto);
        }
    }
    sem_post(&m);
}


void *eseguiCamper(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int lavaggi = 0;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //distribisco nel tempo l'arrivo dei thread
    sleep(rand()%DELAY);

    inizioLavaggioCamper();
    printf("[CAMPER %d]\t\tSotto i rulli dell'autolavaggio\n",*pi);

    sleep(1);                   //lavaggio...

    fineLavaggioCamper();
    lavaggi ++;

    *ptr = lavaggi;                     //ritorna il numero di lavaggi effettuati --> non essendoci ciclo puo' essere solo 0 o 1
    pthread_exit((void *) ptr);
}


void *eseguiAuto(void *id) {
    int *pi = (int *) id;
    int *ptr;
    int lavaggi = 0;
    ptr = (int *) malloc(sizeof(int));

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //distribisco nel tempo l'arrivo dei thread

    sleep(rand()%DELAY);

    inizioLavaggio();
    printf("[AUTO %d]\t\tSotto i rulli dell'autolavaggio\n",*pi);

    sleep(1);                   //lavaggio...
    fineLavaggio();
    lavaggi ++;

    *ptr = lavaggi;                      //ritorna il numero di lavaggi effettuati --> non essendoci ciclo puo' essere solo 0 o 1
    pthread_exit((void *) ptr);
}


int main (int argc, char **argv) {
    pthread_t *thread_auto;
    pthread_t *thread_camper;

    int *taskidsA, *taskidsC;           //taskidsA --> taskid delle auto   taskidsC --> taskid dei camper
    int i;
    int *p;
    int NUM_AUTO, NUM_CAMPER;
    char error[250];

    // Controllo sul numero di parametri
    if (argc != 3 ){ // Devono essere passati 2 parametri
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    // Calcoliamo i numeri passati che saranno il numero di Pthread da creare
    NUM_AUTO = atoi(argv[1]);
    NUM_CAMPER = atoi(argv[2]);

    if (NUM_AUTO <= 0 || NUM_CAMPER <= 0){
        sprintf(error, "Errore: Il primo o secondo parametro non e' un numero strettamente maggiore di 0\n");
        perror(error);
        exit(2);
    }

    //inizializziamo la struttura
    autolavaggio_init();

    //inizializziamo il seme per la rand()
    srand(time(NULL));

    //allocazione memoria per i thread
    thread_auto=(pthread_t *) malloc((NUM_AUTO) * sizeof(pthread_t));
    thread_camper=(pthread_t *) malloc((NUM_CAMPER) * sizeof(pthread_t));

    if (thread_auto == NULL || thread_camper == NULL){
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }

    //allocazione memoria per i taskids
    taskidsA = (int *) malloc(NUM_AUTO * sizeof(int));
    taskidsC = (int *) malloc(NUM_CAMPER * sizeof(int));

    if (taskidsA == NULL || taskidsC == NULL){
        perror("Problemi con l'allocazione dell'array taskidsA o taskidsC\n");
        exit(4);
    }

    // creazione prima delle auto....
    for (i=0; i < NUM_AUTO; i++){
        taskidsA[i] = i;
        if (pthread_create(&thread_auto[i], NULL, eseguiAuto, (void *) (&taskidsA[i])) != 0){
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsA[i]);
            perror(error);
            exit(5);
        }
    }

    //..poi dei camper
    for (i= 0; i < NUM_CAMPER ; i++){
        taskidsC[i] = i;
        if (pthread_create(&thread_camper[i], NULL, eseguiCamper, (void *) (&taskidsC[i])) != 0){
            sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskidsA[i]);
            perror(error);
            exit(5);
        }
    }


    //attendiamo la terminazione delle auto....
    for (i=0; i < NUM_AUTO; i++){
        int *ris;
        pthread_join(thread_auto[i], (void**) & p);
        ris= p;
        printf("Auto %d restituisce %d <-- numero di volte che e' stata lavata\n", i, *ris);
    }

    //...e dei camper
    for (i=0; i < NUM_CAMPER; i++){
        int ris;
        pthread_join(thread_camper[i], (void**) & p);
        ris= *p;
        printf("Camper %d restituisce %d <-- numero di volte che e' stato lavato\n", i, ris);
    }

    exit(0);
}



