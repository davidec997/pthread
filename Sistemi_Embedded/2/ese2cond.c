//
// Created by dada on 23/10/21.
//
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t m;
pthread_cond_t sa;
pthread_cond_t sb;
pthread_cond_t sr;
int na, nb, ba,bb, br,nr;

void startProcA(void){
    pthread_mutex_lock(&m);

    while(nr ||br){
        ba ++;
        pthread_cond_wait(&sa,&m);
        ba--;
    }
        na++;

    pthread_mutex_unlock(&m);
}

void endProcA(void){
    pthread_mutex_lock(&m);
    na--;
    if(br && !nb){
        pthread_cond_signal(&sa);
    }

    pthread_mutex_unlock(&m);

}

void startReset(void){
    pthread_mutex_lock(&m);
    while(na || nb){
        br ++;
        pthread_cond_wait(&sr,&m);
        br --;
    }
    nr++;
    pthread_mutex_unlock(&m);
}

void endReset(void){
    pthread_mutex_lock(&m);
    nr--;
    if(ba)
        pthread_cond_signal(&sa);
    if(bb)
        pthread_cond_signal(&sb);

    pthread_mutex_unlock(&m);
}