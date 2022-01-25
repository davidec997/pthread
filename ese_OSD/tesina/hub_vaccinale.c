
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define DOTTORI 3                                               //numero di dottori disponibili
#define NTIMES 3                                                //numero di vaccini da effettuare
#define DELAY 10                                                //usato per distribuire nel tempo l'arrivo dei cittadini

sem_t m;                                                        //mutex
sem_t s_dosi [3];                                               //ogni dose ha il suo semaforo
int  dosi_blocc [3];                                            //ogni dose ha la sua variabile che indica il numero di cittadini in attesa per quella dose
int dottori_liberi;                                             //indica il num di dottori liberi
char *elenco_dosi [3] = {"PRIMA", "SECONDA", "TERZA"};          //per stampe di controllo

void myInit(void){
    sem_init(&m,0,1);

    for (int i = 0; i < 3; ++i) {
        sem_init(&s_dosi[i], 0, 0);
        dosi_blocc [i] = 0;
    }

    dottori_liberi = DOTTORI;
}

void okVaccino (int * pi, int dose){
    //se posso fare il vaccino --> decremento il num di dottori liberi e faccio la post sul semaforo della dose corrispondente
    dottori_liberi --;
    sem_post(&s_dosi[dose]);
    //sem_wait(&m);
    printf("[THREAD %d]\t\tOk per la %s dose.\n", *pi, elenco_dosi[dose]);
    printf("\n[SERVICE]\t\t Dottori liberi: %d\n", dottori_liberi);
    //sem_post(&m);
}

void notOkVaccino ( int * pi, int dose){
    //se non posso fare il vaccino --> registro che sono bloccato
    dosi_blocc[dose] ++;
    //sem_wait(&m);
    printf("[THREAD %d]\t\tBloccato per la %s dose.\n", *pi,elenco_dosi[dose]);
    printf("\n[SERVICE]\t\tBloccati per la %s dose: %d\n",elenco_dosi[dose], dosi_blocc[dose]);
    //sem_post(&m);
}

void vaccino (int * pi,int dose){
    // le persone che devono fare la prima dose hanno la precedenza su chi deve fare la seconda, e chi deve fare la seconda ha precedenza su quelli che devono fare la terza
    sem_wait(&m);
    switch (dose) {
        case 0:                                                     //prima dose
            if (dottori_liberi > 0 )                                //controllo solo che ci sia un dottore libero
                okVaccino(pi,dose);                                 //posso vaccinarmi
            else
                notOkVaccino(pi, dose);                             // non posso vaccinarmi
            break;
        case 1:                                                     //seconda dose
            if (dottori_liberi > 0 && dosi_blocc[0] == 0)           //controllo che ci sia un dottore e che nessuno sta aspettando la prima dose
                okVaccino(pi,dose);
            else
                notOkVaccino(pi,dose);
            break;
        case 2:
            if (dottori_liberi > 0 && dosi_blocc[0] == 0 && dosi_blocc[1] == 0)         //controllo che ci sia un dottore e che nessuno sta aspettando la prima e seconda dose
                okVaccino(pi, dose);
            else
                notOkVaccino(pi, dose);
            break;
    }

    sem_post(&m);
    sem_wait(&s_dosi[dose]);                                        //wait sulla dose che devo fare

}

void fine_vaccino (){
    // quando qualcuno ha finito di fare il vaccino deve svegliare chi e' in attesa e ha precedenza
    sem_wait(&m);
    dottori_liberi ++;

    for (int i = 0; i < 3; ++i) {
        if (dosi_blocc[i] > 0){                         //appena trovo una coda non vuota..
            dottori_liberi --;                          //decremento il num di dott liberi
            sem_post(&s_dosi[i]);                       //sveglio il primo in fila per quella dose
            dosi_blocc[i] --;                           //e lo tolgo dalla fila
            printf("[SERVICE]\t\tHo svegliato qualcuno che deve fare la %s dose\n\n",elenco_dosi[i]);
            break;
        }
    }
    sem_post(&m);
}


void *eseguiVaccini(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));

    int dose = 0;

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //  ogni persona deve fare 3 dosi
    for(int t = 0; t<NTIMES; t++){
        sleep(rand() % DELAY);

        printf("[THREAD %d]\t\tDevo fare la %s dose del vaccino\n",*pi,elenco_dosi[dose]);
        vaccino(pi, dose);
        sleep(3);           //tempo vaccino

        printf("[THREAD %d]\t\tMi sto vaccinando....\n",*pi);
        fine_vaccino();
        printf("[THREAD %d]\t\tHo fatto la %s dose del vaccino\n\n",*pi,elenco_dosi[dose]);
        dose ++;
    }

    *ptr = dose;                        //ritorna il numero di dosi fatte
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv){
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS;
    char error[250];

    // Controllo sul numero di parametri
    if (argc != 2 ){
        // Deve essere passato esattamente un parametro
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    // Calcoliamo il numero passato che sara' il numero di Pthread da creare
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0){
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
    }

    //init variabili globali
    myInit();
    //init seme per rand()
    srand(time(NULL));

    //allocazione memoria per i thread
    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL){
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    //allocazione memoria per i takids
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL){
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    //crezione dei thread
    for (i=0; i < NUM_THREADS; i++){
        taskids[i] = i;
        if (pthread_create(&thread[i], NULL, eseguiVaccini, (void *) (&taskids[i])) != 0){
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
    }

    for (i=0; i < NUM_THREADS; i++){
        int ris;
        //attendiamo la terminazione di tutti i thread
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d  --> numero di volte che si e' vaccinato\n", i, ris);
    }

    exit(0);
}




