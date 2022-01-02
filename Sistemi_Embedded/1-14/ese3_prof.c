//
// Created by davide on 02/01/22.
//
#define USA_SEM

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// numero buste della mailbox
#define N 3

// scrittori
#define M 8

// lettori
#define R 5

// il dato contenuto nella busta
typedef int T;

#define NESSUNO -1

/* Nota: in questo esempio e' stata enfatizzata la separazione tra
   gestore e struttura dati condivisa. tutto poteva tranquillamente
   essere implementato come una sola struttura busta che al suo
   interno conteneva sia il dato T che l'indice del successivo
   elemento next */



#ifdef USA_SEM

/* Gestore */

/* il gestore e' la struttura dati + le funzioni che regolano
   l'accesso ad una struttura dati condivisa.

   In particolare, questo gestore gestisce un pool di N risorse
   equivalenti, allocandole in una coda concatenata.

   L'accodamento deve essere FIFO; si accoda quando si esegue il punto
   3 della send.

   Nota: l'ordine di accodamento può essere diverso dall'ordine di
   richiesta!!!
*/

struct semaforoprivato_t {
    sem_t s;
    int c;
};

struct gestore_t {

    /* Domandina: questo esempio propone un mutex unico per tutto il
       gestore... in realta', le funzioni devono eseguire in mutua
       esclusione solo a coppie, en in particolare:

       - (il punto 1 della send + 3 della receive)
       - (il punto 3 della send + 1 della receive)

       Perche'???
       Come si fa a sfruttare il parallelismo tra le due coppie?
    */
    sem_t mutex;

    int next[N];
    int head, tail; /* il valore -1 e' utilizzato per terminare la coda */
    int free;

    struct semaforoprivato_t priv[3];

    struct semaforoprivato_t ricezione;
};

void semaforoprivato_init(struct semaforoprivato_t *s)
{
    sem_init(&s->s,0,0);
    s->c = 0;
}

void gestore_init(struct gestore_t *g)
{
    int i;

    /* mutua esclusione */
    sem_init(&g->mutex,0,1);

    /* semafori privati */
    for (i=0; i<3; i++)
        semaforoprivato_init(&g->priv[i]);
    semaforoprivato_init(&g->ricezione);

    /* coda */
    g->head = g->tail = NESSUNO; // non c'e' nessuno in coda

    /* la coda inizialmente e' concatenata tramite free */
    g->free = 0;
    for (i=0; i<N-1; i++)
        g->next[i] = i+1;
    g->next[N-1] = NESSUNO;
}

int gestore_richiedi_busta_vuota(struct gestore_t *g, int prio)
{
    int miblocco;
    int bustavuota;

    sem_wait(&g->mutex);

    /* devo capire se posso accedere alla coda */
    switch (prio) {
        case 0:
            miblocco = g->priv[0].c;
            break;

        case 1:
            miblocco = g->priv[0].c || g->priv[1].c;
            break;

        case 2:
            miblocco = g->priv[0].c || g->priv[1].c || g->priv[2].c;
    }

    if (miblocco || g->free == NESSUNO) {
        g->priv[prio].c++;
        sem_post(&g->mutex);
        sem_wait(&(g->priv[prio].s));
        g->priv[prio].c--;
    }

    bustavuota = g->free;
    g->free = g->next[g->free];

    sem_post(&g->mutex);
    return bustavuota;
}

void gestore_accoda_busta_piena(struct gestore_t *g, int b)
{
    sem_wait(&g->mutex);

    /* inserisco una busta in coda */
    g->next[b] = NESSUNO;
    if (g->head == NESSUNO)
        g->head = b;
    else
        g->next[g->tail] = b;
    g->tail = b;

    if (g->ricezione.c)
        sem_post(&g->ricezione.s);
    else
        sem_post(&g->mutex);
}

int gestore_estrai_busta_piena(struct gestore_t *g)
{
    int bustaestratta;

    sem_wait(&g->mutex);

    if (g->head == NESSUNO) {
        g->ricezione.c++;
        sem_post(&g->mutex);
        sem_wait(&g->ricezione.s);
        g->ricezione.c--;
    }

    /* estraggo una busta */
    bustaestratta = g->head;
    g->head = g->next[g->head];

    sem_post(&g->mutex);

    return bustaestratta;
}

void gestore_rilascio_busta_vuota(struct gestore_t *g, int b)
{
    sem_wait(&g->mutex);

    // rilascio la busta
    g->next[b] = g->free;
    g->free = b;

    // sveglio in modo prioritario
    if (g->priv[0].c)
        sem_post(&g->priv[0].s);
    else if (g->priv[1].c)
        sem_post(&g->priv[1].s);
    else if (g->priv[2].c)
        sem_post(&g->priv[2].s);
    else
        sem_post(&g->mutex);
}

#endif


/* mailbox */

/* la mailbox e' la struttura dati condivisa che permette di inviare e
   ricevere messaggi. la sua struttura dati condivisa b e' un array di
   buste che contengono un tipo T */

struct busta_t {
    T data;
} gestore;


struct mailbox_t {
    struct busta_t b[N];

    struct gestore_t G;
} mailbox;



/* inizializzazione della struttura condivisa */
void init_mailbox(struct mailbox_t *m)
{
    gestore_init(&m->G);

    /* eventuale inizializzazione di tutti gli elementi di b */
}

void send(struct mailbox_t *m, T msg, int prio)
{
    int bustadariempire;

    bustadariempire = gestore_richiedi_busta_vuota(&m->G, prio);
    m->b[bustadariempire].data = msg;
    fprintf(stderr, "  s %8d\n", msg);
    gestore_accoda_busta_piena(&m->G, bustadariempire);
}

T receive(struct mailbox_t *m)
{
    int bustadaritornare;
    T dato;

    bustadaritornare = gestore_estrai_busta_piena(&m->G);
    dato = m->b[bustadaritornare].data;
    fprintf(stderr, "   r %7d\n", dato);
    gestore_rilascio_busta_vuota(&m->G, bustadaritornare);

    return dato;
}


/* ------------------------------- */

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



// un contatore
int cont;



/* i thread */


void *mittente(void *arg)
{
    int p = (int)arg; // Priorita'
    T i;

    for (;;) {
        i = ++cont;
        fprintf(stderr, "S %4d%6d\n", p, i);
        send(&mailbox, cont, p);
        pausetta();
    }
    return 0;
}

void *ricevente(void *arg)
{
    T i;

    for (;;) {
        i = receive(&mailbox);
        fprintf(stderr, " R%10d\n", i);
        pausetta();
    }
    return 0;
}



/* la creazione dei thread */



int main()
{
    pthread_attr_t a;
    pthread_t p;
    int i;

    /* inizializzo il mio sistema */
    init_mailbox(&mailbox);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (i=0; i<M; i++)
        pthread_create(&p, &a, mittente, (void *)(rand()%3));

    for (i=0; i<R; i++)
        pthread_create(&p, &a, ricevente, NULL);


    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(3);

    return 0;
}


