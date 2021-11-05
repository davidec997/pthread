//NON VA
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define _GNU_SOURCE
#define CARTA 0
#define SASSO 1
#define FORBICE 2
char *nomi_mosse[3] = {"carta", "sasso", "forbice"};
sem_t sem_arbitro, sem_g1, sem_g2;
pthread_mutex_t m=PTHREAD_MUTEX_INITIALIZER;
int mossag1,mossag2;
int primo_wait, secondo_wait;
pthread_cond_t cond;
int valGlobale;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}
void myInit(void)
{
   /* sem_init(&sem_arbitro,0,1);
    sem_init(&sem_g1,0,1);
    sem_init(&sem_g2,0,0);
    sem_init(&m,0,1);*/
    pthread_cond_init(&cond,NULL);

    mossag1 = mossag2 =-1;

    primo_wait=0;
    secondo_wait=1;
}

int decreta_vincitore(int g1, int g2){
    printf("allora g1 vale %d e g2 vale %d\n",g1,g2);
    switch (g1) {
        case 0:
            if(g2 == 0) return 0;
            if(g2 == 1) return 1;
            if(g2 == 2) return 2;
            break;
        case 1:
            if(g2 == 0) return 2;
            if(g2 == 1) return 0;
            if(g2 == 2) return 1;
            break;
        case 2:
            if(g2 == 0) return 1;
            if(g2 == 1) return 2;
            if(g2 == 2) return 0;
            break;
    }
}

/*void *arbitro(void *arg)
{
    while(1){
        pthread_mutex_lock(&m);
        printf("A");
        pthread_mutex_unlock(&m);
        //sleep(1);

        // pausetta();
    }


}*/

void *giocatore1(void *arg)
{
    while(1) {
        pthread_mutex_lock(&m);
        if (primo_wait) pthread_cond_wait(&cond,&m);
        primo_wait=1;
        for (int i = 0; i < 200; i++) printf("1\t");
        pthread_cond_signal(&cond);
        secondo_wait = 0;
        pthread_mutex_unlock(&m);

        //sleep(1);
            //pausetta();

    }
}
void *giocatore2(void *arg)
{
    while(1) {
        pthread_mutex_lock(&m);
        if (secondo_wait) pthread_cond_wait(&cond,&m);
        secondo_wait=1;
        for (int i = 0; i < 200; i++) printf("2\t");
        pthread_cond_signal(&cond);
        primo_wait = 0;
        pthread_mutex_unlock(&m);
        //sleep(1);
        //pausetta();

    }
}


/*
int main ()
{ pthread_t th;
    printf("var globale main %d\n",valGlobale);
    sleep(1);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&m, NULL);
    primo_wait=0; */
/* all'inizio Primo non deve aspettare Secondo *//*

    secondo_wait=1;
    valGlobale=0;
    printf("var globale main %d\n",valGlobale);

    pthread_create( &th, NULL, giocatore1, NULL);
    pthread_create( &th, NULL, giocatore2, NULL);
    pthread_join(&th,NULL);
    pthread_join(&th,NULL);

    pthread_exit( NULL );
}
*/

int main() {
    pthread_attr_t a;
    pthread_t p;
    myInit();

    // inizializzo i numeri casuali, usati nella funzione pausetta
    srand(555);

    pthread_attr_init(&a);

   //  non ho voglia di scrivere 10000 volte join!
    // pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    //pthread_create(&p, &a, arbitro, 0);
    pthread_create(&p, &a, giocatore1, 1);
    pthread_create(&p, &a, giocatore2, 1);


    pthread_attr_destroy(&a);

    // aspetto 10 secondi prima di terminare tutti quanti \
    sleep(10);
    //pthread_join(&p,NULL);
    pthread_join(&p,NULL);
    pthread_join(&p,NULL);


}
