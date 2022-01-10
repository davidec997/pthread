//
// Created by dada on 10/01/22.
//
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define N 5
#define CLIENTI 15

struct officina_t{
    pthread_cond_t operai[N], riparazione_ok,coda_clienti;
    pthread_mutex_t m;
    int in_attesa, cliente_in;
}officina;

void init_officina(struct officina_t *officina){
    int i;
    pthread_cond_init(&officina->m,NULL);
    pthread_cond_init(&officina->coda_clienti,NULL);
    for (int j = 0; j < N; j++) pthread_cond_init(&officina->operai[j],NULL);
    officina->in_attesa = officina->cliente_in = 0;

}

void cliente_arrivo(struct officina_t *officina, int riparazione){
    pthread_mutex_lock(&officina->m);

    while (officina->cliente_in > 0){
        officina->in_attesa ++;
        printf("[CLIENTE] ASPETTO PER ENTRARE\n");

        pthread_cond_wait(&officina->coda_clienti,&officina->m);
        officina->in_attesa ++;
        printf("[CLIENTE] ENTRO\n");

    }

    printf("[CLIENTE] RICHIESTA RIPARAZIONE %d\n",riparazione);

    pthread_mutex_unlock(&officina->m);
    pthread_cond_signal(&officina->operai[riparazione]);
}

void cliente_attesafineservizio(struct officina_t *officina){
    pthread_mutex_lock(&officina->m);
    pthread_cond_wait(&officina->riparazione_ok,&officina->m);
    printf("[CLIENTE]  RIPARAZIONE COMPLETA\n");
    if (officina->in_attesa > 0) pthread_cond_signal(&officina->coda_clienti);
    pthread_mutex_unlock(&officina->m);
}

void operaio_attesacliente(struct officina_t *officina, int riparazione){
    // queste print le ho messe nello scritto, ma commentandole l'output può essere più leggibile
    //fprintf(stderr, "%d, aspetto un cliente\n", riparazione);
    pthread_mutex_lock(&officina->m);
    pthread_cond_wait(&officina->operai[riparazione],&officina->m);
    printf("[OPERAIO] RIPARAZIONE %d\n",riparazione);
    pthread_mutex_unlock(&officina->m);    //fprintf(stderr, "%d, servo un cliente\n", riparazione);
}

void operaio_fineservizio(struct officina_t *officina){
    pthread_mutex_lock(&officina->m);
    pthread_cond_signal(&officina->riparazione_ok);
    printf("[OPERAIO] RIPARAZIONE FATTA \n");

    pthread_mutex_unlock(&officina->m);
}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void *cliente(void *arg){
    int r;

    r = rand() % N;
    fprintf(stderr, "arriva nell'ufficio per effettuare una riparazione %d\n", r);
    cliente_arrivo(&officina, r);
    fprintf(stderr, "leggo un giornale\n");
    cliente_attesafineservizio(&officina);
    fprintf(stderr, "torno a casa\n\n");

    return 0;
}

void *operaio(void *arg){
    int r;
    r = *(int*) arg;
    for(;;){
        operaio_attesacliente(&officina, r);
        fprintf(stderr, "riparo %d\n", r);
        pausetta();
        operaio_fineservizio(&officina);
        fprintf(stderr, "pausa\n");
    }
    return 0;
}

int main(int argc, char const *argv[]) {
    pthread_attr_t a;
    pthread_t p;
    int i;
    int num[N];
    init_officina(&officina);

    srand(555);

    pthread_attr_init(&a);

    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for(i=0;i<N;i++){
        num[i] = i;
        pthread_create(&p, &a, operaio, (void *)num + i*sizeof(int));
    }

    for(i=0;i<CLIENTI;i++)
        pthread_create(&p, &a, cliente, NULL);

    pthread_attr_destroy(&a);

    sleep(1);

    return 0;
}

