#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define DELAY 20000
#define NUMFILOSOFI 5
/* stati del filosofo */

#define PENSA 0
#define HAFAME 1
#define MANGIA 2

/* variabili da proteggere */
int statoFilosofo[NUMFILOSOFI];
pthread_mutex_t mutex; /* permette l'accesso alle variabili di stato */
pthread_cond_t condFilosofo[NUMFILOSOFI];

void myInit(void)
{

    pthread_mutex_init(&mutex,NULL);

    for (int i = 0; i <NUMFILOSOFI; i++) pthread_cond_init(&condFilosofo[i],NULL);

    //c_AB = c_Reset = b_AB = b_Reset = 0;
}

int indiceasinistra ( int indice) {
    return( (indice+1)%NUMFILOSOFI );
}
int indiceadestra ( int indice) {
    if( indice==0 ) return( NUMFILOSOFI-1 );
    else return( (indice-1)%NUMFILOSOFI );
}
/* funzione che deve essere eseguita in mutua esclusione */
int puomangiare( int indice ) {
    if (
            statoFilosofo[indice]==HAFAME
            &&
            statoFilosofo[ indiceadestra(indice) ] != MANGIA
            &&
            statoFilosofo[ indiceasinistra(indice) ] != MANGIA
            )
        return( 1 );
    else
        return( 0 );
}

void *filosofo (void *arg) {
    int indice = (int)arg;//indice del filosofo, PASSATO DALL'ARGOMENTO
    printf("Sono il filosofo %d ...\n",indice);
     for(;;) { /* PENSA */
         for (int i = 0; i < DELAY; ++i) {
             
         }
        pthread_mutex_lock(&mutex);
        /* sono affamato, i vicini mi risveglieranno quando finiranno di mangiare */
        statoFilosofo[indice]=HAFAME;
        if ( puomangiare( indice ) ) {
            statoFilosofo[indice] = MANGIA;
            printf("sono il filosofo %d e sto MAGNANDO...\n", indice);
            for (int i = 0; i < DELAY; ++i) ;

        }
        else pthread_cond_wait( &(condFilosofo[indice]), &mutex );
        pthread_mutex_unlock (&mutex );
        /* ora posso prendere le due forchette, so che sono libere */
/* qui ora mangio ..... */
/* rilascio le forchette */
        pthread_mutex_lock(&mutex);
        /* dico che ho smesso di mangiare e non mi servono piu' le forchette */
        statoFilosofo[indice]=PENSA;
        printf("sono il filosofo %d e sto PENSANDO...\n", indice);

        /* cerco di svegliare i vicini se hanno fame e se possono mangiare */
        if ( puomangiare( indiceadestra(indice) ) ) {
            pthread_cond_signal( &(condFilosofo[indiceadestra(indice)]) );
            statoFilosofo[ indiceadestra(indice) ]=MANGIA;
            printf("sono il filosofo %d e sto mangiando...\n", indiceadestra(indice));
        }
        if ( puomangiare( indiceasinistra(indice) ) ) {
            pthread_cond_signal( &(condFilosofo[indiceasinistra(indice)]) );
            statoFilosofo[ indiceasinistra(indice) ]=MANGIA;
            printf("sono il filosofo %d e sto mangiando...\n", indiceasinistra(indice));

        }
        pthread_mutex_unlock(&mutex );
    }
}

int main() {
    pthread_attr_t a;
    pthread_t p;

 inizializzo il sistema

    myInit();

 inizializzo i numeri casuali, usati nella funzione pausetta

    srand(555);

    pthread_attr_init(&a);

 non ho voglia di scrivere 10000 volte join!

    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, filosofo, 0);
    pthread_create(&p, &a, filosofo, 1);
    pthread_create(&p, &a, filosofo, 2);
    pthread_create(&p, &a, filosofo, 3);
    pthread_create(&p, &a, filosofo, 4);

    pthread_attr_destroy(&a);

 aspetto 10 secondi prima di terminare tutti quanti

    sleep(10);

}
