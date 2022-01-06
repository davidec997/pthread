#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define N 10

typedef int T;

typedef struct {
    T mess;
    int prio;
}Busta;




struct Mailbox{
    Busta bustaP[N];
    Busta bustaN[N];
    int tailN;
    int headN;
    int tailP;
    int headP;
    int nElementiN;
    int nElementiP;

};

struct Mailbox mailbox={.tailN=0, .headN=0, .nElementiN=0, .tailP=0, .headP=0, .nElementiP=0};



sem_t senderP, senderN;
sem_t receiver;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;


//IO PRIMA LIBERO IL MUTEX E POI FACCIO LA POST
//NON POSSO OPERARE CON IL MUTEX LOCKATO
//IO PRIMA LIBERO IL MUTEX E POI FACCIO LA WAIT

void controlloMessaggi(struct Mailbox *m){
    T mess;
    T prio;

    if(m->nElementiN==0 && m->nElementiP==0){
        pthread_mutex_unlock(&mutex);
        sem_wait(&receiver);
    }if(m->nElementiP==N){
        mess=m->bustaP[m->tailP].mess;
        prio=m->bustaP[m->tailP].prio;
        //printf("\n \nLA TAIL E' %d\n \n",m->tailP);
        m->tailP=(m->tailP+1)%N;
        m->nElementiP--;
        printf("\nIl messaggio e' %d con priorita %d \n ",mess,prio);
        sem_post(&senderP);
        pthread_mutex_unlock(&mutex);
    }else if(m->nElementiN==N){
        mess=m->bustaN[m->tailN].mess;
        prio=m->bustaN[m->tailN].prio;
        //printf("\n \nLA TAIL E' %d\n \n",m->tailN);
        m->tailN=(m->tailN+1)%N;
        m->nElementiN--;
        printf("\nIl messaggio e' %d con priorita %d \n ",mess,prio);
        sem_post(&senderN);
        pthread_mutex_unlock(&mutex);
    }

    if(m->nElementiP !=0){
        mess=m->bustaP[m->tailP].mess;
        prio=m->bustaP[m->tailP].prio;
        //printf("\n \nLA TAIL E' %d\n \n",m->tailP);
        m->tailP=(m->tailP+1)%N;
        m->nElementiP--;
        printf("\nIl messaggio e' %d con priorita %d \n ",mess,prio);
        pthread_mutex_unlock(&mutex);

    }else if(m->nElementiN !=0){
        mess=m->bustaN[m->tailN].mess;
        prio=m->bustaN[m->tailN].prio;
        m->tailN=(m->tailN+1)%N;
        m->nElementiN--;
        printf("\nIl messaggio e' %d con priorita %d \n ",mess,prio);
        pthread_mutex_unlock(&mutex);
    }

}


void *EseguiRicevente(void *id){
    int *ID=(int *)id;
    sleep(2);
    while(1){
        sem_wait(&receiver);
        pthread_mutex_lock(&mutex);
        controlloMessaggi(&mailbox);
        sleep(5);
    }
}

void SendN(struct Mailbox *m, int ID){

    pthread_mutex_lock(&mutex);

    if(m->nElementiN == N){
        printf("\n Mi sono addormentato senza priorita con nelementi %d\n",m->nElementiN);
        pthread_mutex_unlock(&mutex);
        sem_wait(&senderN);
    }
    m->bustaN[m->headN].mess=ID;
    m->bustaN[m->headN].prio=0;
    printf("Sono il thread %d ed ho scritto un messaggio %d con priorita %d nel posto HEAD:%d \n",ID,m->bustaN[m->headN].mess,m->bustaN[m->headN].prio,m->headN);
    m->headN=(m->headN+1)%N;
    m->nElementiN++;
    sem_post(&receiver);

    pthread_mutex_unlock(&mutex);

}

void SendP(struct Mailbox *m, int ID){

    pthread_mutex_lock(&mutex);

    if(m->nElementiP == N){

        printf("\n Mi sono addormentato senza priorita con nelementi %d\n",m->nElementiP);
        pthread_mutex_unlock(&mutex);
        sem_wait(&senderP);
        pthread_mutex_lock(&mutex);
    }

    m->bustaP[m->headP].mess=ID;
    m->bustaP[m->headP].prio=1;
    printf("Sono il thread %d ed ho scritto un messaggio %d con priorita %d nel posto HEAD:%d \n",ID,m->bustaP[m->headP].mess,m->bustaP[m->headP].prio, m->headP);
    m->headP=(m->headP+1)%N;
    m->nElementiP++;
    sem_post(&receiver);

    pthread_mutex_unlock(&mutex);


}


void *EseguiMittente(void *id){
    int P=rand()%2;
    int *ID=(int *)id;

    sleep(2);
    while(1){


        if(P==1){

            SendP(&mailbox,*ID);
        }else{
            SendN(&mailbox,*ID);
        }

        sleep(5);

    }
}



int main(int argc, char **argv){


    pthread_t *thread;
    int i, *p, *taskids;
    int NUM_MITTENTI;
    int NUM_RICEVENTI;
    int NUM_THREADS;

    if (sem_init(&senderN, 0, 0) == -1){
        printf("Errore inizializzazione del semaforo senderN");
        exit(0);
    }

    if (sem_init(&senderP, 0, 0) == -1){
        printf("Errore inizializzazione del semaforo senderP");
        exit(1);
    }

    if(sem_init(&receiver,0,0)== -1){
        printf("Errore inizializzazione  del semaforo receiver");
        exit(2);
    }




    if (argc =! 3 ) /* Deve essere passato nessun un parametro */
    {
        printf("Errore nel numero dei parametri -> Inserire Numero Mittenti e Numero Riceventi");
        exit(1);
    }

    NUM_MITTENTI=atoi(argv[1]);
    NUM_RICEVENTI=atoi(argv[2]);

    NUM_THREADS=NUM_MITTENTI+NUM_RICEVENTI;


    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    taskids=(int *) malloc(NUM_THREADS * sizeof(int));



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




    for (i=0; i < NUM_MITTENTI; i++)
    {

        taskids[i] = i;
        printf("Sto per creare il thread %d-esimo mittente\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, EseguiMittente, (void *) (&taskids[i])) != 0){
            printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread cannibale %d-esimo \n", taskids[i]);
        }

    }


    for (; i < NUM_THREADS; i++)
    {

        taskids[i] = i;
        printf("Sto per creare il thread %d-esimo ricevente\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, EseguiRicevente, (void *) (&taskids[i])) != 0){
            printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread cannibale %d-esimo \n", taskids[i]);
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

