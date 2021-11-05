
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int PrimoFaiWait;
int SecondoFaiWait;
int valGlobale=0; /* dati da proteggere in sezione critica */
int flag;
typedef enum {false,true} Boolean;

void *Primo ( void * arg ) {
    for(int i = 0; i< 100;i++) {
        pthread_mutex_lock(&mutex);
/* se Secondo ha gia' fatto la signal allora Primo deve proseguire senza fare la wait */
        if ( flag ) pthread_cond_wait(&cond,&mutex);
/* al prossimo giro Primo devo fare la wait,
 a meno che Secondo non faccia la signal prima che Primo tenti di fare la wait */
        flag = false;
/* SEZIONE CRITICA : legge cio' che e' stato messo da Secondo e lo cambia */
        for(int t=0;t<200000;t++);
        printf("1\t");

        pthread_cond_signal(&cond); /* risveglio Secondo */
/* Nel caso che Secondo non abbia ancora fatto la wait allora Primo dice a Secondo
 che non deve aspettare perche' Primo ha gia' fatto la signal */
        pthread_mutex_unlock(&mutex); /* rilascio mutua esclusione, cosi' Secondo parte */
    }
}

void *Secondo ( void *arg ) {
    for(int i = 0; i< 100;i++) {
        pthread_mutex_lock(&mutex);
        if ( !flag ) pthread_cond_wait(&cond,&mutex);
        flag = true;
        for(int t=0;t<200000;t++);
        printf("2 \t");
        pthread_cond_signal(&cond); /* risveglio Primo */
        pthread_mutex_unlock(&mutex); /* rilascio mutua esclusione, così Primo può partire */
    }
}

int main ()
{
    pthread_t th;
    // sleep(1);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mutex, NULL);
    //PrimoFaiWait=0; /* all'inizio Primo non deve aspettare Secondo */
    //SecondoFaiWait=1;

    printf("var globale dddd %d\n",valGlobale);

    pthread_create( &th, NULL, Primo, NULL);
    pthread_create( &th, NULL, Secondo, NULL);
    pthread_join(&th,NULL);
    pthread_join(&th,NULL);

    pthread_exit( NULL );
}