#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define L 3                                             //lettini disponibili
#define DELAY 20                                        //max ritardo d'esecuzione per i thread

typedef enum {false,true} Boolean;
int  pazienti_blocc;                                    //indica il numero di pazienti in attesa
int  donatori_blocc;                                    //indica il numero di donatori in attesa
sem_t s_pazienti, s_donatori,m;                         //semafori privati per donatori e pazinenti e mutex
sem_t attesa, lettino;                                  //semafori generali per segnalare al dottore che c'e' almeno un cliente e un lettino libero
int lettini_liberi;                                     //tiene il conto dei lettini liberi

void myInit(void){
    sem_init(&m,0,1);
    sem_init(&s_pazienti, 0, 0);
    sem_init(&s_donatori, 0, 0);
    sem_init(&attesa, 0, 0);
    sem_init(&lettino, 0, L);                           //inizializzato con il numero di lettini disponibili

    lettini_liberi = L;                                 //inizializzato con il numero di lettini disponibili
    pazienti_blocc = donatori_blocc = 0;
}

void inizio_prelievo(int *pi, Boolean tipo){
    sem_post(&attesa);                                      //post su attesa per 'svegliare' il dottore se addormentato

    sem_wait(&m);
    if(tipo){                                               //a seconda del tipo (0-->donatore   1-->paziente)...
        printf("[PAZIENTE %d]\t\tIn attesa per prelievo\n",*pi);
        pazienti_blocc ++;                                  //..segno che c'e' un paziente in attesa...

    } else {
        printf("[DONATORE %d]\t\tIn attesa per prelievo\n",*pi);
        donatori_blocc ++;                                  //.. o un donatore in attesa
    }
    sem_post(&m);

    if (tipo)
        sem_wait(&s_pazienti);                              //aspetto l'ok dal dottore, sempre in base al tipo di cliente
    else
        sem_wait(&s_donatori);

}

void prelievo (int *pi, Boolean tipo){
    sem_wait(&m);                                           //solo per stampe di controllo
    if (tipo)
        printf("[PAZIENTE %d]\t\tSTO FACENDO IL PRELIEVO\n",*pi);
    else
        printf("[DONATORE %d]\t\tSTO FACENDO IL PRELIEVO\n",*pi);
    sem_post(&m);
}

void fine_prelievo (int *pi, int tipo) {
//ogni cliente aspetta ancora alcuni secondi per riprendersi e poi libera il lettino...
    sleep(rand()%4 +1);
    sem_wait(&m);
    lettini_liberi ++;                                       //incrementa lettini disponibili
    if (tipo)
        printf("[PAZIENTE %d]\t\tME NE VADO...\n",*pi);
    else
        printf("[DONATORE %d]\t\tME NE VADO...\n",*pi);
    sem_post(&m);
    sem_post(&lettino);                                     //e fa una post su lettino
}

void esegui_prelievo(){
    //stampa di controllo
    printf("\n[SERVICE]\t\tDonatori bloccati %d\tpazienti bloccati %d\tlettini liberi %d\n",donatori_blocc,pazienti_blocc,lettini_liberi);

    if(donatori_blocc == 0 && pazienti_blocc == 0)                  //stampo il messaggio solo il dottore si blocca perche' non ci sono ne donatori ne pazienti
        printf("[MEDICO]\t\tAttesa cliente\n");

    sem_wait(&attesa);                                              //il dottore attende un cliente

    if(lettini_liberi == 0)                                          //stampo il messaggio solo il dottore si blocca perche' non ci sono lettini
        printf("[MEDICO]\t\tAttesa lettino\n");

    sem_wait(&lettino);                                             //il dottore attende un lettino

    //il dottore da precedenza ai donatori, solo se non ce ne sono allora sveglia un paziente
    sem_wait(&m);
    printf("[MEDICO]\t\tinizio prelievo");

    if (donatori_blocc > 0){
        sem_post(&s_donatori);
        donatori_blocc --;
        printf(" ad un donatore\n");

    } else if (pazienti_blocc > 0){
        sem_post(&s_pazienti);
        pazienti_blocc --;
        printf(" ad un paziente\n");
    }

    lettini_liberi --;                                      //decrementa il num di lettini liberi...
    sem_post(&m);

    sleep(1);                                               //..ed esegue il prelievo

    sem_wait(&m);                                           //mutex solo per stampa di controllo
    printf("[MEDICO]\t\tfine prelievo\n");
    sem_post(&m);

}

void *eseguiMedico(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));

    while (1){
        esegui_prelievo();
    }

    *ptr = *pi;
    pthread_exit((void *) ptr);
}

void *eseguiCliente(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    Boolean tipo;
    int prelievi = 0;                       //numero di prelievi effettuati
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    tipo = rand() % 2;                      //0-->donatore   1-->paziente
    sleep(rand()%DELAY);                    //distribuisco nel tempo l'arrivo dei clienti
    inizio_prelievo(pi,tipo);

    prelievo(pi,tipo);

    fine_prelievo(pi,tipo);
    prelievi ++;

    *ptr = prelievi;                      //non essendoci ciclo, vale al massimo 1
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
    if (argc != 2 ) {
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

    //inizializzo le variabili condivise
    myInit();

    //init del seme per la rand()
    srand(time(NULL));

    //allocazione spazio per thread
    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL){
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }

    //allocazione spazio per il vettore dei taskids
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL){
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    //creazione thread
    for (i=0; i < NUM_THREADS; i++){
        taskids[i] = i;
        if (i == NUM_THREADS -1)        //l'ultimo thread e' il dottore
            if (pthread_create(&thread[i], NULL, eseguiMedico, (void *) (&taskids[i])) != 0){
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(5);
            }
        if (pthread_create(&thread[i], NULL, eseguiCliente, (void *) (&taskids[i])) != 0){
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
    }

    for (i=0; i < NUM_THREADS - 1; i++){
        int ris;
        // attendiamo la terminazione di tutti i thread clienti ma non del dottore
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d  --> numero di prelievi\n", i, ris);
    }

    exit(0);
}







