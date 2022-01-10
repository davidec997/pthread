//FUNZIONA
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define  M 5
#define  NTIMES 5
int NUM_THREADS;

typedef enum {false, true} Boolean;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int a_att,b_att,c_att,d_att;
int a_bloc, b_bloc, c_bloc, d_bloc;
int turno;
sem_t s_a, *s_b, s_c, s_d, sych;

void myInit(){
    a_att=b_att=c_att=d_att = 0;
    a_bloc = b_bloc = c_bloc = d_bloc = 0;
    s_b = malloc(NUM_THREADS * sizeof (sem_t));
    for (int i = 0; i < NUM_THREADS; i++)     sem_init(&s_b[i],0,0);

    sem_init(&s_a,0,0);
    sem_init(&s_c,0,0);
    sem_init(&s_d,0,0);
    sem_init(&sych,0,0);
    turno = 0;
}

void pausetta(void){
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void A(int * pi){
    pthread_mutex_lock(&m);
    if (a_att == 0){
        a_att ++;
        sem_post(&s_a);
    } else a_bloc ++;

    pthread_mutex_unlock(&m);
    sem_wait(&s_a);

    printf("[THREAD %d]\t\tSto eseguendo A\n",*pi);
    sleep(1);

    pthread_mutex_lock(&m);
    a_att--;
    if (a_bloc > 0 && a_att == 0){
        sem_post(&s_a);
        a_bloc --;
        a_att ++;
    }
    pthread_mutex_unlock(&m);


}
void B(int * pi){
    pthread_mutex_lock(&m);
    if (turno == *pi && b_att == 0){
        b_att ++;
        sem_post(&s_b[*pi]);
    } else b_bloc ++;

    pthread_mutex_unlock(&m);
    sem_wait(&s_b[*pi]);

    printf("[THREAD %d]\t\tSto eseguendo B\n",*pi);
    pausetta();

    pthread_mutex_lock(&m);
    b_att --;
    turno = (turno + 1) % NUM_THREADS;
    sem_post(&s_b[turno]);
    if (b_bloc) b_bloc--;
    b_att ++;
    pthread_mutex_unlock(&m);

}
void C(int * pi){
    pthread_mutex_lock(&m);
    if (d_att == 0){
        sem_post(&s_c);
        c_att ++;
    } else c_bloc ++;

    pthread_mutex_unlock(&m);
    sem_wait(&s_c);

    printf("[THREAD %d]\t\tSto eseguendo C\n",*pi);
    sleep(1);
    pthread_mutex_lock(&m);
    c_att --;
    if (c_att == 0){
        if (d_bloc > 0){
            while (d_bloc > 0){
                d_att ++;
                d_bloc --;
                sem_post(&s_d);
            }
        }
    }
    pthread_mutex_unlock(&m);


}
void D(int * pi){
    pthread_mutex_lock(&m);
    if (c_att == 0){
        sem_post(&s_d);
        d_att ++;
    } else d_bloc ++;

    pthread_mutex_unlock(&m);
    sem_wait(&s_d);

    printf("[THREAD %d]\t\tSto eseguendo D\n",*pi);
    sleep(1);
    pthread_mutex_lock(&m);
    d_att --;
    if (d_att == 0){
        if (c_bloc > 0){
            while (c_bloc > 0){
                c_att ++;
                c_bloc --;
                sem_post(&s_c);
            }
        }
    }
    pthread_mutex_unlock(&m);
}
void fineciclo(int * pi){
    pausetta();
    if (*pi == NUM_THREADS -1){
        printf("Sono il thread %d e sveglio tutti\n",*pi);
        for (int i = 0; i < NUM_THREADS; ++i) sem_post(&sych);
    }
    printf("[THREAD %d] Si blocca su synch",*pi);
    sem_wait(&sych);
    printf("[THREAD %d]\t\tSi Sblocca da synch",*pi);

}

void *esegui(void *id)
{
    int *pi = (int *)id;
    int *ptr;
    int iterazione,porzioni_prese = 0;

    ptr = (int *) malloc( sizeof(int));
    if (ptr == NULL)
    {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    for (int i = 0; i < NTIMES; i++) {

        printf("[THREAD %d]\t\t\tCODICE PRIVATO 1\n",*pi);
        pausetta();
        A(pi);
        printf("[THREAD %d]\t\t\tCODICE PRIVATO 2\n",*pi);
        pausetta();
        B(pi);
        printf("[THREAD %d]\t\t\tCODICE PRIVATO 3\n",*pi);
        if(rand()%2) D(pi);
        else C(pi);
        printf("[THREAD %d]\t\t\tCODICE PRIVATO 4\n",*pi);
        pausetta();

        fineciclo(pi);

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
    NUM_THREADS = atoi(argv[1]);
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

    //pthread_create(&thread[0], NULL, eseguiCuoco, (void *) (&taskids[0]));
    // addetto
    sleep(1);

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        // printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, esegui, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        //printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

    for (i=1; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d \n", i, ris);
    }

    pthread_mutex_destroy(&m);
    exit(0);
}





