
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>


pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t prelievo=PTHREAD_COND_INITIALIZER;
pthread_cond_t attesa=PTHREAD_COND_INITIALIZER;

int bloccati;
int richiesta;
int saldo;
int operazione;
int valore;

void Prelievo(int ID, int valore){

    if(bloccati>0){
        pthread_cond_wait(&attesa,&mutex);
    }

    if(saldo-valore<0){
        printf("Sono il cliente  %d e volevo prelevare %d. Sono in attesa. Operazione negata: Credito insufficiente: %d\n",ID,valore,saldo);
        bloccati++;
        richiesta=valore;
        pthread_cond_wait(&prelievo,&mutex);
    }

    printf("Sono il cliente %d e STO PRELEVANDO: %d. Valore conto prima del prelievo: %d\n",ID,valore,saldo);

    saldo=saldo-valore;
    sleep(2);
    printf("Sono il cliente %d ed HO PRELEVATO: %d Valore conto dopo il prelievo: %d\n\n",ID,valore,saldo);

}

void Deposito(int ID, int valore){
    printf("Sono il cliente %d e STO DEPOSITANDO: %d. Valore conto prima del deposito: %d\n",ID,valore,saldo);
    saldo=saldo+valore;
    sleep(2);

    printf("Sono il cliente %d HO DEPOSITATO: %d. Valore conto dopo il deposito: %d\n",ID,valore,saldo);

    if(saldo>richiesta){
        bloccati--;
        pthread_cond_signal(&prelievo);
        pthread_cond_signal(&attesa);
    }
}

void *EseguiUtente(void *id){
    int *ID=(int*)id;
    while(1){

        pthread_mutex_lock(&mutex);
        operazione=rand()%2;

        valore=rand()%600;
        if(operazione==0){
            Prelievo(*ID,valore);
        }else{
            Deposito(*ID,valore);
        }

        pthread_mutex_unlock(&mutex);
        sleep(5);
    }
}


int main(int argc, char **argv){

    pthread_t *thread;
    int i, *p, *taskids;
    int NUM_THREADS;

    if (argc != 2 )
    {
        printf("Errore nel numero dei parametri -> Inserire un parametro");
        exit(1);
    }

    NUM_THREADS=atoi(argv[1]);

    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    taskids=(int *) malloc(NUM_THREADS * sizeof(int));

    saldo=100;
    bloccati=0;

    if (thread == NULL)
    {
        printf("Problemi con l'allocazione dell'array thread\n");
        exit(2);
    }

    taskids = (int *) malloc(NUM_THREADS * sizeof(int));

    if (taskids == NULL)
    {
        printf("Problemi con l'allocazione dell'array taskids per tenere traccia del numero del thread\n");
        exit(3);
    }


    for (i=0; i < NUM_THREADS; i++)
    {

        taskids[i] = i;
        printf("Sto per creare il cliente : %d\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, EseguiUtente, (void *) (&taskids[i])) != 0){
            printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread uomo %d-esimo \n", taskids[i]);
        }

    }

    for (i=0;i < NUM_THREADS; i++)
    {
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo ha restituito %d \n", i, ris);
    }

    exit(6);

}

