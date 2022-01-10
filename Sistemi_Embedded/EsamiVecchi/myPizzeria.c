
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>


#define N 11 // posti in pizzeria


typedef enum {false, true} Boolean;
int NUM_THREADS;


struct pizzeria_t {
   sem_t pizzaiolo, cliente[N], m;
   int ordini [N];
   int icc;     // indice cliente corrente
   int in_attesa;
}pizzeria;

void init_pizzeria (struct pizzeria_t *p){

    sem_init(&p->m,0,1);
    sem_init(&p->pizzaiolo,0,0);
    for (int i = 0; i < N; i++){
        sem_init(&p->cliente[i],0,0);
        p->ordini[i] = 0;
    }
    p->icc = -1;
    p->in_attesa = 0;
}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*3000000;
    nanosleep(&t,NULL);
}

void ordinaPizze (struct pizzeria_t * p, int numerocliente, int npizze){

    sem_wait(&p->m);

    printf("[CLIENTE %d]\tORDINA %d PIZZE\n",numerocliente, npizze);
    p->ordini[numerocliente] = npizze;
    if (p->in_attesa == 1){
        printf("SVEGLIO IL PIZZAIOLO\n");
        p->in_attesa --;
        p->icc = numerocliente;
        sem_post(&p->pizzaiolo);
    } else
        sem_post(&p->m);

}

void ritiraPizze (struct  pizzeria_t * p , int numerocliente) {
    sem_wait(&p->cliente[numerocliente]);

    /*sem_wait(&p->m);
    printf("[CLIENTE %d]\t\tHO LE MIE PIZZE VADO A MANGIARE\n",numerocliente);
   // p->in_attesa --;
    sem_post(&p->m);*/
}



void *cliente (void *arg) {
    int *pi = (int *)arg;
    int *ptr;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    //senza ciclo
    sleep(rand()%4 + 1);
    printf("[CLIENTE %d]\tSono alla pizzeria...\n",*pi);
    //printf("[CLIENTE %d]\tSono entrato in pizzeria...Scelgo le pizze\n",*pi);
    ordinaPizze(&pizzeria,*pi, (rand()%9) + 1);
    //printf("[CLIENTE %d]\tHo ordinato le pizze...\n",*pi);
    //sleep(2);
    ritiraPizze(&pizzeria,*pi);
    printf("[CLIENTE %d]\tSto ANDANDO A MANGIARE LE PIZZE...\n",*pi);



    *ptr = *pi;
    pthread_exit((void *) ptr);
}


void prossima_pizza(struct pizzeria_t *p) {
    int i, min , cliente,v;
    sem_wait(&p->m);

   /* if (p->in_attesa == 0){
        printf("PIZZAIOLO SI BLOCCA XK NON CI SONO CLIENTI\n");// se non ci sono clienti mi blocco
        sem_post(&p->m);
        sem_wait(&p->pizzaiolo);
        sem_wait(&p->m);
    }*/
    printf("\tSITUA\t");
    for (int i = 0; i < N; i++) printf("%d\t",p->ordini[i]);
    printf(" indice : %d\n",p->icc);

    if (p->icc == -1){
        // mi serve l'indice
        min = 0;
        cliente = -1;
        for (int i = 0; i < N; i++) {       // trovo il primo > 0
            if(p->ordini[i] > 0){
                cliente = i;
                min = p->ordini[i];
                break;
            }
        }

        while (i < N){
            if(p->ordini[i] < min && p->ordini[i] > 0){
                min = p->ordini[i];
                cliente = i;
            }
            i ++;
        }
         if (min == 0){
             printf("PIZZAIOLO SI BLOCCA XK NON CI SONO CLIENTI\n");// se non ci sono clienti mi blocco
             p->in_attesa ++;
             sem_post(&p->m);
             sem_wait(&p->pizzaiolo);
             //sem_wait(&p->m);
         } else
             p->icc = cliente;
        // ho l'indice



    }

    printf("[PIZZAIOLO] SERVE %d con pizze %d\n",p->icc,p->ordini[p->icc]);
    sem_post(&p->m);
    //faccio una pizza
}

void consegna_pizza(struct pizzeria_t *p){

    sem_wait(&p->m);
    p->ordini[p->icc] --;

    if (p->ordini[p->icc] == 0){
        sem_post(&p->cliente[p->icc]);
        printf("[PIZZAIOLO]\t\t\tHO COMPLETATO L'ORDINE DEL CLIENTE %d\n",p->icc);
        p->icc = -1;
    }
    sem_post(&p->m);
}

void *pizzaiolo (void * arg){
    int *pi = (int *)arg;
    int *ptr;
    int ord;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL){
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    sleep(2);

    while (1){
        //printf("[PIZZAIOLO]\t\tciaa\n");
        prossima_pizza(&pizzeria);
        //cuoci pizzza
        sleep(1);
        consegna_pizza(&pizzeria);
    }


    *ptr = *pi;
    pthread_exit((void *) ptr);
}

int main (int argc, char **argv)
{
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 2 ) /* Deve essere passato esattamente un parametro */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS = N;
    if (NUM_THREADS <= 0)
    {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
    }

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

    init_pizzeria(&pizzeria);
    srand(555);

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        if (i == NUM_THREADS -1) pthread_create(&thread[i], NULL, pizzaiolo, (void *) (&taskids[i]));
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, cliente, (void *) (&taskids[i])) != 0)
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
        printf("Pthread %d-esimo restituisce %d \n", i, ris);
    }

    exit(0);
}




