#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include<stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>




#define CARTA 0
#define SASSO 1
#define FORBICE 2


char *nomi_mosse[3] = {"carta", "sasso", "forbice"};

sem_t Arbitro1;
sem_t Arbitro2;
sem_t Giocatore1;
sem_t Giocatore2;
int Mossa1,Mossa2,Vincente, Pareggio,g=0;
char scelta;



void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void generaMossa1(){
    Mossa1=1;
}



void generaMossa2(){
    Mossa2=0;
    printf("%d",Mossa2);
}

void confronto_mosse(){
    if(Mossa1==CARTA && Mossa2==SASSO){
        Vincente=Mossa1;
        g=1;
    }else if(Mossa1==CARTA && Mossa2==FORBICE){
        Vincente=Mossa2;
        g=2;
    }else if(Mossa1==SASSO && Mossa2==CARTA){
        Vincente=Mossa2;
        g=2;
    }else if(Mossa1==SASSO && Mossa2==FORBICE){
        Vincente=Mossa1;
        g=2;
    }
}

void *EseguiArbitro(void *id){
    printf("sto qio");
    while(1){
        printf("ston nel while 1 dell'arbitro");
        sem_post(&Giocatore1);
        sem_post(&Giocatore2);

        sem_wait(&Arbitro1);

        sem_wait(&Arbitro2);


        confronto_mosse();
        printf("\n Il giocatore 1 ha giocato %s \n",nomi_mosse[Mossa1]);
        printf("\n Il giocatore 2 ha giocato %s \n",nomi_mosse[Mossa2]);
        if(g==1){
            printf("\n Il vincitore è giocatore 1 con mossa %s \n",nomi_mosse[Vincente]);
        }else if(g==2){
            printf("\n Il vincitore è giocatore 2 con mossa %s \n",nomi_mosse[Vincente]);
        }else if(Pareggio==1){
            printf("\n E' finita in pareggio! \n");
        }

        sleep(20);
        scanf("Premere un tasto per giocare %c",&scelta);
    }

}

void *EseguiGiocatore1(void *id){
    printf("CIAO gioc1");
    while(1){

        sem_wait(&Giocatore1);


        generaMossa1();

        printf("Sono il player 1 ed ho fatto la mossa %s",nomi_mosse[Mossa1]);

        sem_post(&Arbitro1);
    }
}




void *EseguiGiocatore2(void *id){
    while(1){

        sem_wait(&Giocatore1);

        generaMossa2();
        printf("Sono il player 1 ed ho fatto la mossa %s",nomi_mosse[Mossa2]);
        sem_post(&Arbitro2);
    }
}




int main (int argc, char **argv)
{
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS=3;





    if (sem_init(&Arbitro1, 0, 0) == -1)
        write(1,"Errore inizializzazione semaforo", 33);

    if (sem_init(&Arbitro2, 0, 0) == -1)
        write(1,"Errore inizializzazione semaforo", 33);

    if (sem_init(&Giocatore1, 0, 0) == -1)
        write(1,"Errore inizializzazione semaforo", 33);

    if (sem_init(&Giocatore2, 0, 0) == -1)
        write(1,"Errore inizializzazione semaforo", 33);

    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));

    if (thread == NULL)
    {
        printf("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }

    taskids = (int *) malloc(NUM_THREADS * sizeof(int));

    if (taskids == NULL)
    {
        printf("Problemi con l'allocazione dell'array taskids per tenere traccia del numero del thread\n");
        exit(4);
    }


    taskids[0]=0;
    taskids[1]=1;
    taskids[2]=2;

    printf("Sto per creare il thread arbitro \n");

    if (pthread_create(&thread[0], NULL, EseguiArbitro, (void *) (&taskids[0])) != 0)
        printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[0]);


    printf("Sto per creare il thread giocatore1 \n");
    if (pthread_create(&thread[1], NULL, EseguiGiocatore1, (void *) (&taskids[1])) != 0)
        printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[1]);


    printf("Sto per creare il thread giocatore2\n ");
    if (pthread_create(&thread[2], NULL, EseguiGiocatore2, (void *) (&taskids[2])) != 0)
        printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[2]);


    //sleep(5);

    pthread_join(&thread[0],NULL);
    pthread_join(&thread[1],NULL);
    pthread_join(&thread[2],NULL);


}
