#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define CARTAINA 0
#define TABACCO 2
#define FIAMMIFERI 2
char *nomi[3] = {"cartina", "tabacco", "fiammiferi"};
int tavolo [3];
sem_t sem_tabaccaio, sem_f1, sem_f2,sem_f3,m;
int tabaccaio_ok,fumatore1_ok,fumatore2_ok,fumatore3_ok,turno;


void myInit(void)
{
    for (int i = 0; i < 3; i++) tavolo[i]=0;
    sem_init(&sem_tabaccaio, 0, 1);
    sem_init(&sem_f1, 0, 0);
    sem_init(&sem_f2, 0, 0);
    sem_init(&sem_f3, 0, 0);

    sem_init(&m,0,1);

    fumatore1_ok = fumatore2_ok = fumatore3_ok = 0;
    tabaccaio_ok =1;
    turno=0;
}

void svegliaFumatore(){
    if (!tavolo[0]){
        // il primo elemento e' 0 quindi il tabaccaio ha messo sul tavolo gli altri 2
        //allora posso sevgliare il fumatore 0 che con gli ele 1 e 2 puo fumare
        printf("\tTABACCAIO SVEGLIA IL FUMATORE 1\n");
        sem_post(&m);
        sem_post(&sem_f1);
    }else if (!tavolo[1]){
        printf("\tTABACCAIO SVEGLIA IL FUMATORE 2\n");
        sem_post(&m);
        sem_post(&sem_f2);
    }else if (!tavolo[2]){
        printf("\tTABACCAIO SVEGLIA IL FUMATORE 3\n");
        sem_post(&m);
        sem_post(&sem_f3);
    }
    //return;
}

void *tabaccaio(void *arg){
    int id = (int) arg;
    int e1,e2;

    while(1){
        printf("\nSono il tabaccaio e sto per dare mettere sul tavolo 2 elementi..\n");
        sem_wait(&sem_tabaccaio);
        sem_wait(&m);
        e1 = rand() % 3;
        e2 = rand() % 3;
        if (e2 == e1) e2 = (e2 + 1) % 3;

        printf("Metto sul tavolo l'elemento %s e %s \n",nomi[e1],nomi[e2]);
        tavolo[e1]=1;
        tavolo[e2]=1;
        printf("\tSITUAZIONE TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        // il tabaccaio sa quali ele ha messo sul tavolo e sveglia il fortunato che ha il terzo ele
        svegliaFumatore();
        sleep(2);
    }
}

void *fumatore1(void *arg)
{
    int id = (int) arg;
    printf("\tSONO IL FUMATORE %d e HO SOLO %s\n",id,nomi[id]);

    while(1){
        sem_wait(&sem_f1);
        sem_wait(&m);
        //tolgo dal tavolo i 2 elementi
       // printf("\tSITUAZIONE [f1 a] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        tavolo[1] = 0;
        tavolo[2] = 0;
       // printf("\tSITUAZIONE [f1 b] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);

        printf("~~~~~~~~~~~FUMATORE %d STA FUMANDO~~~~~~~~~~~\n",id+1);
        sem_post(&m);
        //faccio la post al tabaccaio
        sleep(1);
        sem_post(&sem_tabaccaio);
    }

}

void *fumatore2(void *arg)
{
    int id = (int) arg;
    printf("\tSONO IL FUMATORE %d e HO SOLO %s\n",id,nomi[id]);

    while(1){
        sem_wait(&sem_f2);
        sem_wait(&m);
        //tolgo dal tavolo i 2 elementi
       // printf("\tSITUAZIONE [f2 a] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        tavolo[0] = 0;
        tavolo[2] = 0;
        printf("~~~~~~~~~~~FUMATORE %d STA FUMANDO~~~~~~~~~~~\n",id+1);
        //printf("\tSITUAZIONE [f2 b] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        sem_post(&m);
        //faccio la post al tabaccaio
        sleep(1);
        sem_post(&sem_tabaccaio);
    }

}

void *fumatore3(void *arg)
{
    int id = (int) arg;
    printf("\tSONO IL FUMATORE %d e HO SOLO %s\n",id,nomi[id]);

    while(1){
        sem_wait(&sem_f3);
        sem_wait(&m);
        //tolgo dal tavolo i 2 elementi
       // printf("\tSITUAZIONE [f3 a] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        tavolo[0] = 0;
        tavolo[1] = 0;
        printf("~~~~~~~~~~~FUMATORE %d STA FUMANDO~~~~~~~~~~~\n",id+1);
        //printf("\tSITUAZIONE [f3 b] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        sem_post(&m);
        //faccio la post al tabaccaio
        sleep(1);
        sem_post(&sem_tabaccaio);
    }

}


int main(int argc, char *argv[]) {

    pthread_t *thread;
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
    NUM_THREADS =  4;//atoi(argv[1]);
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

    myInit();
    srand(555);

    pthread_create(&thread[0], NULL, tabaccaio, (void *) (&taskids[0]));
    // addetto
    sleep(3);

    pthread_create(&p, NULL, tabaccaio, 99);
    pthread_create(&p, NULL, fumatore1, 0);
    pthread_create(&p, NULL, fumatore2, 1);
    pthread_create(&p, NULL, fumatore3, 2);

    for (i=1; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    sleep(10);  // il programma termina in base al timer anche se avrebbe dovuto girare all'infinito dato che ci sono solo while(1)
    exit(0);

}

