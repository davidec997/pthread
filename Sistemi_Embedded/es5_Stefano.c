//
// Created by davidec on 26/12/21.
// es 5 --> stefano
//
/* OBIETTIVO: generare un numero non noto di threads (il numero specifico viene passato come parametro al main) che eseguono la stampa del loro numero d'ordine e del loro identificatore, quindi tornano (solo per prova) il valore 1000 a cui viene sommato il loro numero d'ordine (che e' passato all'atto della loro creazione) ==> i thread NON interagiscono in alcun modo fra di loro (se non per l'uso dello standard output) */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t sem_ris;

int NUM_THREADS,contatore=0,nb=0;
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct struct_urna {
    int zero;
    int uno;
} struct_urna;
//typedef struct struct_urna struct_urna;
struct_urna urna;



void vota(int voto)
{
    pthread_mutex_lock(&mutex);
    if (voto==0)
    {
        urna.zero++;
    }
    else
    {
        urna.uno++;
    }
    pthread_mutex_unlock(&mutex);


}

int risultato(int voto)
{
    int verdetto;

    //printf("Il mezzo del numero equivale a %d, il numero di zeri %d e di uno %d \n",(NUM_THREADS/2)+1,zero,uno);
    if ((urna.zero<(NUM_THREADS/2)+1) && urna.uno<(NUM_THREADS/2)+1)
    {
        pthread_mutex_lock(&mutex);
        //printf("-- Qui mi blocco e sono il numero %d \n",nt);
        nb++;
        pthread_mutex_unlock(&mutex);
        sem_wait(&sem_ris);
    }
    else
    {
        pthread_mutex_lock(&mutex);
        //printf("-- Qui mi SBLOCCO sono numero %d mezzo = %d z = %d u = %d \n",nt,(NUM_THREADS/2)+1,zero,uno);
        contatore++;
        pthread_mutex_unlock(&mutex);
        //sem_post(&sem_ris);
    }
    pthread_mutex_lock(&mutex);
    if (contatore==1)
    {
        while(nb>0)
        {
            sem_post(&sem_ris);
            nb--;
        }

    }
    if(urna.zero>urna.uno)
    {
        verdetto=0;
    }
    else
    {
        verdetto=1;
    }
    pthread_mutex_unlock(&mutex);

    return (verdetto);

}



void *funzione_thread(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int voto=rand()%2;
    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    *ptr = 1000+*pi;
    vota(voto);
    printf(" Sono il Thread %d-esimo ed ho votato %d \n", *pi,voto);

    if(voto==risultato(voto))
    {
        printf(" --Ho vinto e sono il numero %d \n",*pi);
    }
    else
    {
        printf(" --Ho perso e sono il numero %d \n",*pi);
    }


    pthread_exit((void *) ptr);
}



int main (int argc, char **argv)
{

    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    contatore=0;
    urna.zero=0;
    urna.uno=0;
    char error[250];
/* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS = 31;
    /* inizializzo il semeb */
    srand(time(NULL));

    //controllo l'inizializzazione del semaforo go
    if (sem_init(&sem_ris, 0, 0)!=0)
    {
        printf("Errore con l'inizializzazione del semaforo go");
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
//printf("iterazione %d\n",i);
    for (i=0;i<NUM_THREADS;i++)
    {
        taskids[i] = i;
        if (pthread_create(&thread[i], NULL, funzione_thread, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN ed ho creato il thread %i-esimo \n", i);
    }
    /* attendiamo la terminazione del thread che causa la fine */
    for (i=0; i < NUM_THREADS; i++)
    {

        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        //printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }
    printf("Risultato finale:\nNumero di zero: %d\nNumero di uno: %d\n",urna.zero,urna.uno);
    exit(0);
}

