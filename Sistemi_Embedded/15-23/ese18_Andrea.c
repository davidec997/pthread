#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

#define NESSUNO 0
#define STATO_F1 1
#define STATO_F2 2
#define STATO_F3 3
#define STATO_F4 4
#define STATO_F5 5
#define STATO_F6 6



sem_t F1,F2, F3, F4, F5, F6;
pthread_mutex_t mutex;
int stato;
int aF1,aF2,aF3,aF4,aF5,aF6;
int bF1,bF2,bF3,bF4,bF5,bF6;
int R=1;

//QUANDO MI DICE MUTUA ESCLUSIONE -> O ENTRA UNA O ENTRA UN'ALTRA
//QUANDO MI DICE CONCORRENTEMENTE -> POSSONO ENTRARE TUTTE PERO' UNA ALLA VOLTA (QUANDO LAVORO SU VARIABILI CONDIVISE, IL MUTEX CI DEVE ESSERE PER FORZA)
//SE AVESSERO VOLUTO ENTRARE TUTTE INSIEME (PASSANO COMUNQUE UNA ALLA VOLTA) AVREI DOVUTO METTERE UN SEMAFORO DI BLOCCO PRIVATO NEL NON VERIFICARSI DELLA CONDIZIONE, DOPO AVER LIBERATO IL MUTEX

void StartF1(int ID){
    pthread_mutex_lock(&mutex);

    if(stato!=STATO_F1 && stato!=NESSUNO){
        printf("Processo: %d bloccato nella F1 a causa di F2 attive\n\n",ID);
        bF1++;

    }else if(stato==NESSUNO){

        aF1++;
        stato=STATO_F1;
        sem_post(&F1);
    }

    pthread_mutex_unlock(&mutex);
    sem_wait(&F1);
    sleep(2);

}



void EndF1(int ID){
    pthread_mutex_lock(&mutex);
    aF1--;

    if(bF3>0){
        stato=STATO_F3;
        bF3--;
        aF3++;
        sem_post(&F3);
    }
    pthread_mutex_unlock(&mutex);
}


void StartF2(int ID){

    pthread_mutex_lock(&mutex);

    if(stato!=STATO_F2 && stato!=NESSUNO){
        printf("Processo: %d bloccato nella F2 a causa di F1 attive\n\n",ID);
        bF2++;

    }else if(stato==NESSUNO){

        aF2++;
        stato=STATO_F2;
        sem_post(&F2);
    }

    pthread_mutex_unlock(&mutex);
    sem_wait(&F2);
    sleep(2);

}



void EndF2(int ID){

    pthread_mutex_lock(&mutex);
    aF2--;
    if(bF3>0){
        stato=STATO_F3;
        bF3--;
        aF3++;
        sem_post(&F3);
    }
    pthread_mutex_unlock(&mutex);
}


void StartF3(int ID){

    pthread_mutex_lock(&mutex);

    if(stato!=STATO_F3){
        printf("Processo: %d bloccato nella F3 a causa di F2 attive o F1 attive\n\n",ID);
        bF3++;
    }else{
        aF3++;
        sem_post(&F3);

    }

    pthread_mutex_unlock(&mutex);
    sem_wait(&F3);
    sleep(2);

}


void EndF3(int ID){

    pthread_mutex_lock(&mutex);

    aF3--;

    if(aF3==0 && bF4>0){
        bF4--;
        aF4++;
        stato=STATO_F4;
        sem_post(&F4);
    }else{
        while(bF3>0){
            bF3--;
            aF3++;
            sem_post(&F3);
        }
    }
    pthread_mutex_unlock(&mutex);
}


void StartF4(int ID){


    pthread_mutex_lock(&mutex);

    if(stato!=STATO_F4){
        printf("Processo: %d bloccato nella F4 a causa di F2 o F3 o F1 attive\n\n",ID);
        bF4++;

    }else{

        aF4++;
        sem_post(&F4);
    }


    pthread_mutex_unlock(&mutex);
    sem_wait(&F4);

}


void EndF4(int ID){

    pthread_mutex_lock(&mutex);
    aF4--;

    if(bF1>0 && aF4==0){
        stato=STATO_F1;
        bF1--;
        aF1++;
        sem_post(&F1);
    }else if(bF2>0 && aF4==0){
        stato=STATO_F2;
        bF2--;
        aF2++;
        sem_post(&F2);
    }
    else{
        stato=NESSUNO;

    }
    pthread_mutex_unlock(&mutex);

}


void StartF5(int ID){

    pthread_mutex_lock(&mutex);

    if(stato!=NESSUNO){
        printf("Processo: %d bloccato nella F5 a causa di F2 o F3 o F1 attive\n\n",ID);
        bF5++;
    }else{
        stato=STATO_F5;
        aF5++;
        bF5--;
        sem_post(&F5);
    }

    pthread_mutex_unlock(&mutex);
    sem_wait(&F5);
}


void EndF5(int ID){
    pthread_mutex_lock(&mutex);
    aF5--;
    if(bF6>0 && aF5==0 ){
        stato=STATO_F6;
        bF6--;
        aF6++;
        sem_post(&F6);
    }

    pthread_mutex_unlock(&mutex);
}


void StartF6(int ID){

    pthread_mutex_lock(&mutex);
    if(stato!=STATO_F6){
        printf("Processo: %d bloccato nella F6 a causa di F5 attive\n\n",ID);
        bF6++;
    }else{

        stato=STATO_F6;
        aF6++;
        bF6--;
        sem_post(&F6);
    }

    pthread_mutex_unlock(&mutex);
    sem_wait(&F6);
}



void EndF6(int ID){

    pthread_mutex_lock(&mutex);
    aF6--;
    if(bF5>0 && aF6==0){
        aF5++;
        bF5--;
        sem_post(&F5);
    }else{

        stato=NESSUNO;
    }

}






void *processi(void *id){

    int *ID=(int*)id;
    int procedura;

    srand(time(NULL));
    procedura=rand()%6;

    sleep(5);
    while(1){


        if(procedura==0){
            StartF1(*ID);

            printf("Sono il processo: %d in F1 e sto consumando la risorsa R: %d \n",*ID,R);
            EndF1(*ID);
            printf("Sono il processo: %d in F1 ed ho finito con la risorsa: %d	\n",*ID,R);
            sleep(1);
        }else if(procedura==1){
            StartF2(*ID);
            printf("Sono il processo: %d in F2 e sto consumando la risorsa R: %d \n",*ID,R);
            EndF2(*ID);
            printf("Sono il processo: %d ed F2 ho finito con la risorsa: %d\n",*ID,R);
            sleep(1);
        } else if(procedura==2){
            StartF3(*ID);
            printf("Sono il processo: %d in F3 e sto consumando la risorsa R: %d \n",*ID,R);
            EndF3(*ID);
            printf("Sono il processo: %d ed F3 ho finito con la risorsa: %d\n",*ID,R);
            sleep(1);
        }else if(procedura==3){
            StartF4(*ID);
            printf("Sono il processo: %d in F4 e sto consumando la risorsa R: %d \n",*ID,R);
            EndF4(*ID);
            printf("Sono il processo: %d ed F4 ho finito con la risorsa: %d\n",*ID,R);
            sleep(1);
        } if(procedura==4){
            StartF5(*ID);
            printf("Sono il processo: %d in F5 e sto consumando la risorsa R: %d \n",*ID,R);
            EndF5(*ID);
            printf("Sono il processo: %d ed F5 ho finito con la risorsa: %d\n",*ID,R);
            sleep(1);
        } else if(procedura==5){
            StartF6(*ID);
            printf("Sono il processo: %d in F6 sto consumando la risorsa R: %d \n",*ID,R);
            EndF6(*ID);
            printf("Sono il processo: %d ed F6 ho finito con la risorsa: %d\n",*ID,R);
            sleep(1);
        }

    }
    sleep(3);
}



int main (int argc, char **argv)
{
    pthread_t *Thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS;




    if(argc!=2){
        printf("Errore inserire un parametro\n");
        exit(1);
    }


    NUM_THREADS=atoi(argv[1]);

    sem_init(&F1,0,0);
    sem_init(&F2,0,0);
    sem_init(&F3,0,0);
    sem_init(&F4,0,0);
    sem_init(&F5,0,0);
    sem_init(&F6,0,0);

    stato=NESSUNO;
    aF1=aF2=aF3=aF4=aF5=aF6=0;
    bF1=bF2=bF3=bF4=bF5=bF6=0;




    Thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));

    if (Thread == NULL)
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

    for (i=0; i < NUM_THREADS; i++)
    {


        taskids[i] = i;
        //printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&Thread[i], NULL, processi, (void *) (&taskids[i])) != 0){
            sleep(2);
            printf("SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo \n", taskids[i]);
        }else{
            printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu \n", i, Thread[i]);

        }
    }


    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        pthread_join(Thread[i], (void**) & p);
        //ris= *p;
        //printf("Pthread %d-esimo ha restituito %d \n", i, ris);
    }



}











