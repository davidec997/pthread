//
// Created by davide on 27/12/21.
//

//DEADLOCK DOPO 10 ITER
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N 4     //sedie
#define BARBIERI 3

#define NTIMES 5
typedef enum {false, true} Boolean;

int sedie_libere;
pthread_mutex_t m,cassa;
pthread_cond_t cust,cassiere,cust_coda,stop;
pthread_cond_t *barb;

Boolean barbieri_occupati [BARBIERI];
Boolean cassiere_occupato;
int clienti_in_attesa,clienti_fuori;

void myInit(void)
{
    pthread_mutex_init(&m,NULL);
    pthread_mutex_init(&cassa,NULL);

    pthread_cond_init(&cust,NULL);
    pthread_cond_init(&stop,NULL);

    barb = malloc(BARBIERI * sizeof (pthread_cond_t));
    for (int i = 0; i < 3; i++) pthread_cond_init(&barb[i],NULL);
    pthread_cond_init(&cassiere,NULL);
    pthread_cond_init(&cust_coda,NULL);

    sedie_libere = N;
    for (int i =0;i<BARBIERI;i++) barbieri_occupati[i]=false;
    cassiere_occupato = false;
    clienti_in_attesa =  clienti_fuori = 0;

}


void servi( int *pi){
    // wait su barber
    // mi svegliano
    //taglio i capelli
    //signal su cust

    pthread_mutex_lock(&m);
    printf("[BARBIERE %d]\tCiao ora dormo\n",*pi);
    if (clienti_in_attesa <= 0)
        pthread_cond_wait(&cust,&m);

    printf("[BARBIERE %d]\tSono stato svegliato\n",*pi);

    barbieri_occupati[*pi] = true;
    clienti_in_attesa--;
    sedie_libere ++;
    printf("[BARBIERE %d]\tSto servendo un cliente\n",*pi);
    barbieri_occupati[*pi] = false;
    sleep(2);

    pthread_mutex_unlock(&m);
    pthread_cond_signal(&barb[*pi]);
    //pthread_cond_signal( &cust); // notifico a un cliente che ho finito di tagliare i capelli

}

int checkBarber(){
    int index = -1;
    if (barbieri_occupati[0] == false)
        index = 0;
    else if (barbieri_occupati[1] == false)
        index = 1;
    else if (barbieri_occupati[2] == false)
        index = 2;
    return index;
}

void richiedi_servizio(int *pi){
    int * ptr;
    ptr = (int *) malloc(sizeof(int));
    int i;

    pthread_mutex_lock(&m);
    /*while(sedie_libere <= 0) {
        printf("[CLIENTE %d]\t\tTUTTE LE SEDIE OCCUPATE.. APETTO FUORI\n", *pi);
        clienti_fuori++;
        sleep(1);
    }*/
//------------------------------mi accomodo---------------------------------------------------------------------
    clienti_in_attesa ++;
    sedie_libere--;
    printf("[CLIENTE %d]\t\tMI SONO SEDUTO SU UNA SEDIA, controllo se c'e' un barbiere disponibile\n",*pi);

    while ((i=checkBarber()) == -1){
        printf("[CLIENTE %d]\t\tTUTTI I BARBIERI SONO OCCUPATI... ASPETTO\n",*pi);
        pthread_cond_wait(&stop,&m);   //
    }

    if (clienti_in_attesa <= 1)
        pthread_cond_signal(&cust);
    printf("[CLIENTE %d]\t\tIL BARBIERE %d e' libero.. Infatti -->%d  (se 0 non e' occupato)\n",*pi,i,barbieri_occupati[i]);

    pthread_cond_wait(&barb[i],&m);

    printf("[SERVICE]\t\tSEDIE LIBERE %d\t Clienti in attesa %d\n", sedie_libere, clienti_in_attesa);

    //pthread_mutex_unlock(&m);
    //  pthread_cond_signal(&cust); //??
    //pthread_cond_signal(&barb[i]);
    printf("SONO IL CLIENTE %d IL BARBIERE %d MI STA SERVENDO...\n", *pi,i);
    pthread_mutex_unlock(&m);

    sleep(5);
}

void paga (int *pi){
    pthread_mutex_lock(&cassa);
    printf("[CASSIERE]\t\tIL CLIENTE %d STA PAGANDO\n",*pi);
    sleep(1);
    pthread_mutex_unlock(&cassa);

    // facciamo che il cassiere sveglia i clienti fuori dalla porta e li fa sedere sulle poltrone

    pthread_mutex_lock(&m);
    if (sedie_libere > 0){
        // se c'e' una sedia libera sveglio un cliente che asp fuori
        pthread_cond_signal(&cust_coda);
        clienti_fuori --;
    }
    pthread_mutex_unlock(&m);
}

void *customerRoutine(void * id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    printf("Sono il thread cliente %d\n",id);
    for (;;) {
        richiedi_servizio(pi);
        paga(pi);
    }

    *ptr = NULL;
    pthread_exit((void *) ptr);
}



void *barberRoutine(void * id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    int clineti_serviti =1;
    printf("Sono il thread barbiere %d\n",pi);
    for (;;){
        //sleep(1);
        servi(pi);
        printf(" Sono il barbiere e Ho servito  %d clienti ...\n",clineti_serviti);
        clineti_serviti ++;
    }

    *ptr = NULL;
    pthread_exit((void *) ptr);
}


int main (int argc, char **argv)
{
    pthread_t thread;

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

    myInit();

    pthread_create(&thread,NULL,barberRoutine,0);
    pthread_create(&thread,NULL,barberRoutine,1);
    pthread_create(&thread,NULL,barberRoutine,2);

    pthread_create(&thread,NULL,customerRoutine,1);
    pthread_create(&thread,NULL,customerRoutine,2);
    pthread_create(&thread,NULL,customerRoutine,3);
    pthread_create(&thread,NULL,customerRoutine,4);
    pthread_create(&thread,NULL,customerRoutine,5);
    /* for (i=0; i < NUM_THREADS ; i++) {

         taskids[i] = i;
         //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
         if (i == 0) { // il primo thread  e' il barbiere... gli altri sono clienti
             if (pthread_create(&thread[i], NULL, barberRoutine, (void *) (&taskids[i])) != 0) {
                 sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",
                         taskids[i]);
                 perror(error);
                 exit(5);
             }
         } else {
             if (pthread_create(&thread[i], NULL, customerRoutine, (void *) (&taskids[i])) != 0) {
                 sprintf(error, "SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n",
                         taskids[i]);
                 perror(error);
                 exit(5);
             }
             printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
         }
     }*/

    sleep(5);
    /*for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        *//* attendiamo la terminazione di tutti i thread generati *//*
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }*/

    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);

    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);
    pthread_join(&thread, (void**) & p);


    exit(0);
}


