/* OBIETTIVO: generare un numero non noto di threads che eseguono il codice corrispondente al problema della gestione di una mensa:
 * il primo thread deve essere l'addetto e gli altri sono i clienti.
 * Il numero di contenitori dei vassoi e' definito dalla costante M (uguale a 5 ad esempio) e ogni contenitore ha K (uguale a 7 ad esempio) vani: chiaramente, e' possibile variare tale numero a piacimento;
 * si segnala che il caso piu' interessante e' comunque quello in cui il numero di clienti sia significativamente maggiore del numero di vani: ad esempio, 50 clienti.
 * Ogni thread cliente torna al main il numero random generato */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define  M 5
#define  K 7
#define  NTIMES 10
typedef enum {false, true} Boolean;

/* variabili globali */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;      /* semaforo di mutua esclusione per l'accesso a tutte le variabili condivise (simula il semaforo di mutua esclus
ione associato ad una istanza di tipo monitor) */
pthread_cond_t vuoti =  PTHREAD_COND_INITIALIZER;	/* variabile condizione su cui si sospendono i clienti se non ci sono vani disponibili */
pthread_cond_t pieni =  PTHREAD_COND_INITIALIZER;	/* variabile condizione su cui si sospende l'addetto se non ci sono contenitori da vuotare */

int residuo[M];  					/* array che tiene conto dei vani liberi */
int addettoAttivo = false;				/* se true indica che l'addetto e' stato attivato */

int cerca_contenitore(int k);

void TROVA_CONTENITORE_LIBERO(int id, int count)
{
    int trovato;
    /* N.B. count puo' valere 1 o 2  */
    pthread_mutex_lock(&mutex);     /* simulazione di inizio procedura entry del monitor */
    while ((trovato = cerca_contenitore(count)) == -1) /* si deve usare un while ANCHE perche' quando si rilasciano i vani non si puo' sapere le singole richieste fatte da altri clienti */
    {
        /* bisogna svegliare l'addetto se non sono stati trovati contenitori liberi e qualcuno non lo ha gia' svegliato\ */
        if (!addettoAttivo)
        {
            printf("CLIENTE %d: trovato tutti i contenitore pieni e quindi sveglio l'addetto\n", id);
            addettoAttivo=true;
            pthread_cond_signal(&pieni);
        }

        /* quindi ci si sospende in attesa che l'addetto svolga il proprio compito */
        pthread_cond_wait(&vuoti, &mutex);
    }
    printf("CLIENTE %d: trovato %d vani liberi nel contenitore %d-esimo\n", id, count, trovato);
    residuo[trovato] -= count;	/* se abbiamo avuto successo nel trovare il numero di vani sufficienti, allora decrementiamo il numero */
    pthread_mutex_unlock(&mutex);   /* simulazione di termine procedura entry del monitor */
}

void SVUOTA_CONTENITORI()
{
    int j;
    pthread_mutex_lock(&mutex);     /* simulazione di inizio procedura entry del monitor */
    while (!addettoAttivo)
        pthread_cond_wait(&pieni, &mutex);	/* l'addetto si sospende sempre perche' verra' risvegliato solo su necessita' */
    for (j=0; j < M; j++)
    {
        printf("ADDETTO: trovato il contenitore %d-esimo pieno e ora lo svuoto\n", j);
        residuo[j] = K;
    }
    addettoAttivo=false;
    pthread_cond_broadcast(&vuoti); 	/* risveglio tutti i clienti */
    pthread_mutex_unlock(&mutex);   /* simulazione di termine procedura entry del monitor */
}

int cerca_contenitore(int k)
{
    int risultato, j;
    risultato = -1;
    for (j=0; j < M; j++)
    {
        if (residuo[j] >= k)
        {
            risultato = j;
            break;
        }
    }
    return risultato;
}

int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    casuale++;              /* si incrementa dato che la rand produce un numero random fra 0 e n-1, mentre a noi serve un numero fra 1 e n */
    return casuale;
}

void *eseguiCliente(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int r;	/* per generare un numero random di vani da richiedere (compreso fra 1 e 2 */

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        printf("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("CLIENTE-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());
    for (int i = 0; i < NTIMES; ++i) {
        r=mia_random(2);
        TROVA_CONTENITORE_LIBERO(*pi, r);
        printf("Thread%d e identificatore %lu posso posare il mio vassoio\n", *pi, pthread_self());
        printf("Thread%d e identificatore %lu vado a casa\n", *pi, pthread_self());
    }

    /* pthread torna al padre il numero random calcolato */
    *ptr = r;
    pthread_exit((void *) ptr);
}

void *eseguiAddetto(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int i;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        printf("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("ADDETTO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    for (i = 0; ; i++) /* while (true)  l'addetto DEVE essere un ciclo senza fine */
    {
        SVUOTA_CONTENITORI();
        printf("ADDETTO-Ora lavo i piatti\n");
    }

    /* NON SI ARRIVERA' MAI QUI */
    *ptr = *pi;
    pthread_exit((void *) ptr);
}


int main (int argc, char **argv)
{
    int NUM_THREADS;
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;

    /* Controllo sul numero di parametri */
    if (argc != 2 ) /* Deve essere passato esattamente un parametro */
    {
        printf("Errore nel numero dei parametri %d\n", argc-1);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0)
    {
        printf("Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        exit(2);
    }

    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
    {
        printf("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        printf("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    srand(time(NULL)); /* inizializziamo il seme per la generazione random di numeri  */

    /* prima di creare i thread, andiamo ad inizializzare l'array residuo */
    for (i=0; i < M; i++)
        residuo[i] = K;

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        if (i == 0)
        {
            printf("Sto per creare il thread ADDETTO %d-esimo\n", taskids[i]);
            if (pthread_create(&thread[i], NULL, eseguiAddetto, (void *) (&taskids[i])) != 0)
                printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            sleep(5); /* per sicurezza ritardiamo la creazione dei clienti per essere certi che l'addetto sia gia' sospeso quando un cliente non dovesse trovare spazio per il suo vassoio */
        }
        else
        {
            printf("Sto per creare il thread CLIENTE %d-esimo\n", taskids[i]);
            if (pthread_create(&thread[i], NULL, eseguiCliente, (void *) (&taskids[i])) != 0)
                printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
        }
        printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        if (i == 0)
            printf("Dato che l'addetto e' un ciclo infinito NON possiamo aspettarlo\n");
            /* si ricorda che all'exit del main si produrra' la terminazione comunque dell'addetto */
        else
        {
            pthread_join(thread[i], (void**) & p);
            ris= *p;
            printf("CLIENTE-Pthread %d-esimo restituisce %d\n", i, ris);
        }
    }

    exit(0);
}
