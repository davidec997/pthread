#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

struct gestore_t {

    pthread_mutex_t M;
    pthread_mutex_t M1;
    pthread_mutex_t M2;
    pthread_cond_t COND;

    bool occupato1;
    bool occupato2;
}gestore;

void *timer(void*arg)
{
    while(1)
    {
        sleep(1);
        pthread_cond_broadcast(&gestore.COND);
    }

    return 0;
}

int Richiesta(int t, struct gestore_t *g)
{
    int ris;
    pthread_mutex_lock(&g->M);

    if(g->occupato1 && g->occupato2)
    {
        int conta=0;
        printf("aspetto \n");

        while((g->occupato1 && g->occupato2) && conta<t)
        {
            pthread_cond_wait(&g->COND,&g->M);
            conta++;
            printf("	conta=%d t=%d \n",conta,t);
        }
    }

    if(!g->occupato1)
    {
        pthread_mutex_lock(&g->M1);
        g->occupato1=true;
        ris=1;
    }
    else
    if(!g->occupato2)
    {
        pthread_mutex_lock(&g->M2);
        g->occupato2=true;
        ris=2;
    }
    else
        ris=0;

    pthread_mutex_unlock(&g->M);
    return ris;
}

void Rilascio (struct gestore_t *g, int ris)
{
    if(ris==0)
        return;

    pthread_mutex_lock(&g->M);

    if(ris==1)
    {
        g->occupato1=false;
        pthread_mutex_unlock(&g->M1);
    }
    else
    {
        g->occupato2=false;
        pthread_mutex_unlock(&g->M2);
    }

    pthread_mutex_unlock(&g->M);
}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}


void* P(void* arg)
{
    int *id=(int*)arg;
    int ris;
    int t=(rand()%10)+1;

    for(;;)
    {
        ris=Richiesta(t,&gestore);

        printf("P%d ha acquisito risorsa %d \n",*id,ris);
        sleep(3);

        Rilascio(&gestore,ris);

        printf("P%d ha rilasciato la risorsa %d \n",*id,ris);

        pausetta();
    }

    return 0;
}

void init_gestore(struct gestore_t* g)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->M, &m_attr);
    pthread_mutex_init(&g->M1, &m_attr);
    pthread_mutex_init(&g->M2, &m_attr);
    pthread_cond_init(&g->COND, &c_attr);

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);

    g->occupato1=false;
    g->occupato2=false;
}


int main()
{
    pthread_t *thread=(pthread_t*)malloc(5*sizeof(pthread_t));
    int *tid=(int*)malloc(5*sizeof(int));
    pthread_attr_t a;

    srand(555);
    init_gestore(&gestore);

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for(int i=0; i<5; i++)
    {
        tid[i]=i;

        if(i==0)
            pthread_create(&thread[i],&a,timer,(void*)&tid[i]);
        else
            pthread_create(&thread[i],&a,P,(void*)&tid[i]);
    }

    pthread_attr_destroy(&a);

    sleep(20);

    return 0;

}



