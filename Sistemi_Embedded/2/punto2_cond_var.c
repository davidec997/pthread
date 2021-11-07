#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define BUSY 1000000
#define CYCLE 50

pthread_mutex_t m;
pthread_cond_t priv_A,priv_B;
pthread_cond_t priv_Reset;
int c_A,c_B, c_Reset;          // conta le istanze in esecuzione
int b_A, b_B, b_Reset;          // conta il numero dei bloccati

void myInit(void)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&m, &m_attr);
    pthread_cond_init(&priv_A, &c_attr);
    pthread_cond_init(&priv_B, &c_attr);

    pthread_cond_init(&priv_Reset, &c_attr);

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);

    c_A = c_B = c_Reset = b_A = b_B = b_Reset = 0;
}

void StartProcA(void)
{
    pthread_mutex_lock(&m);
    while (c_Reset || b_Reset || c_A) {
        b_A++;
        pthread_cond_wait(&priv_A, &m);
        b_A--;
    }
    c_A++;
    pthread_mutex_unlock(&m);
}

void EndProcA(void)
{
    pthread_mutex_lock(&m);

    c_A--;
    if (b_Reset && !c_A) {
        pthread_cond_signal(&priv_Reset);
    } else if (b_A){
        pthread_cond_signal(&priv_A);
    }
    pthread_mutex_unlock(&m);
}


// le procedure di B si comportano in modo identico a quelle di A
void StartProcB(void)
{
    pthread_mutex_lock(&m);
    while (c_Reset || b_Reset || c_B) {
        b_B++;
        pthread_cond_wait(&priv_B, &m);
        b_B--;
    }
    c_B++;
    pthread_mutex_unlock(&m);
}

void EndProcB(void)
{
    pthread_mutex_lock(&m);

    c_B--;
    if (b_Reset && !c_B) {
        pthread_cond_signal(&priv_Reset);
    } else if(b_B){
        pthread_cond_signal(&priv_B);
    }
    pthread_mutex_unlock(&m);
}

void StartReset(void)
{
    pthread_mutex_lock(&m);
    while (c_A || c_B) {
        b_Reset++;
        pthread_cond_wait(&priv_Reset, &m);
        b_Reset--;
    }
    c_Reset++;
    pthread_mutex_unlock(&m);
}

void EndReset(void)
{
    pthread_mutex_lock(&m);

    c_Reset--;
    if (b_A) {
        pthread_cond_broadcast(&priv_A);
    } else if (b_B){
        pthread_cond_broadcast(&priv_B);

    }
    pthread_mutex_unlock(&m);
}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

/* ------------------------------------------------------------------------ */

/* le funzioni della risorsa R fittizia */


void myprint(char *s,int id)
{
    int i,j;
    fprintf(stderr,"[");
    for (j=0; j<CYCLE; j++) {
        fprintf(stderr,s,id);
        for (i=0; i<BUSY; i++);
    }
    fprintf(stderr,"]");
}
void ProcA(int id)
{
    myprint("%d",id); //-
}

void ProcB(int id)
{
    myprint("%d",id); // +
}

void Reset(int id)
{
    myprint("%d",id); //.
}


void *PA(void *arg)
{
    for (;;) {
        int id = (int) arg;
        fprintf(stderr,"A");
        StartProcA();
        ProcA(id);
        EndProcA();
        fprintf(stderr,"a");
    }
    return 0;
}

void *PB(void *arg)
{
    for (;;) {
        int id = (int) arg;
        fprintf(stderr,"B");
        StartProcB();
        ProcB(id);
        EndProcB();
        fprintf(stderr,"b");
    }
    return 0;
}

void *PR(void *arg)
{
    for (;;) {
        int id = (int) arg;
        fprintf(stderr,"R");
        StartReset();
        Reset(id);
        EndReset();
        fprintf(stderr,"r");
        pausetta();
        pausetta();
    }
    return 0;
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il sistema */
    myInit();

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, PA, 0);
    pthread_create(&p, &a, PA, 1);
    pthread_create(&p, &a, PA, 2);

    pthread_create(&p, &a, PB, 7);
    pthread_create(&p, &a, PB, 8);
    pthread_create(&p, &a, PB, 9);

    pthread_create(&p, &a, PR, 5);

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(5);

    return 0;
}
