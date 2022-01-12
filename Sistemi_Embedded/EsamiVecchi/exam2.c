
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

struct bandierine_t{
    sem_t prontog1,prontog2,startg1, startg2, m, fine;
    int bandierina_presa, salvo, preso;
    int vincitore;
}bandierine;


void init_bandierine (struct bandierine_t *b){
    sem_init(&b->m,0,1);
    sem_init(&b->prontog1,0,0);
    sem_init(&b->prontog2,0,0);
    sem_init(&b->startg1,0,0);
    sem_init(&b->startg2,0,0);
    sem_init(&b->fine,0,0);
    b->bandierina_presa = b->salvo = b->preso = 0;
    b->vincitore = -1;

}

void pausetta(void){
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void attendi_il_via (struct bandierine_t *b, int ngio){
    if (ngio){
        sem_post(&b->prontog2);
        sem_wait(&b->startg2);
        printf("\t\t\tGIOCATORE 2 PARTE\n");
    } else{
        sem_post(&b->prontog1);
        sem_wait(&b->startg1);
        printf("\t\t\tGIOCATORE 1 PARTE\n");
    }
}

int bandierina_presa (struct bandierine_t *b, int ngio){
    int ris = 0;
    //sleep(rand()%3);
    pausetta();


    sem_wait(&b->m);
    if (ngio){
        if (b->bandierina_presa == 0){
            b->bandierina_presa = 2;
            ris = 1;
            printf("[GIOCATORE %d]\tPRENDE LA BANDIERINA\n",ngio +1);
        }
    } else {
        if (b->bandierina_presa == 0){
            b->bandierina_presa = 1;
            ris = 1;
            printf("[GIOCATORE %d]\tPRENDE LA BANDIERINA\n",ngio +1);
        }
    }
    sem_post(&b->m);
    return ris;
}

int sono_salvo (struct bandierine_t *b, int ngio){
    int ris = 0;
    // sleep(rand()%3);
    pausetta();
    pausetta();

    sem_wait(&b->m);
    if (ngio){
        if (b->salvo == 0){
            b->salvo = 1;
            b->preso = -1;
            ris = 1;
            b->vincitore = 2;
            printf("[GIOCATORE %d]\tPRENDE LA BANDIERINA ED E' SALVO!\n",ngio +1);
            sem_post(&b->fine);
        }
    } else {
        if (b->salvo == 0){
            b->salvo = 1;
            b->preso = -1;
            ris = 1;
            b->vincitore = 1;
            printf("[GIOCATORE %d]\tPRENDE LA BANDIERINA ED E' SALVO!\n",ngio +1);
            sem_post(&b->fine);
        }
    }

    sem_post(&b->m);
    return ris;
}

int ti_ho_preso (struct bandierine_t *b, int ngio){
    int ris = 0;
    //sleep(rand()%3);
    pausetta();

    sem_wait(&b->m);
    if (ngio){
        if (b->preso == 0){
            b->salvo = -1;
            b->preso = 1;
            ris = 1;
            b->vincitore = 2;
            printf("[GIOCATORE %d]\tNON HA PRESO LA BANDIERINA MA HA RAGGIUNTO L'ATRO!!\n",ngio +1);
            sem_post(&b->fine);
        }
    } else {
        if (b->preso == 0){
            b->salvo = -1;
            b->preso = 1;
            ris = 1;
            b->vincitore = 1;
            printf("[GIOCATORE %d]\tNON HA PRESO LA BANDIERINA MA HA RAGGIUNTO L'ATRO!!\n",ngio +1);
            sem_post(&b->fine);
        }
    }

    sem_post(&b->m);
    return ris;
}

void *giocatore ( void * arg){
    int numerogicatore = (int) arg;
    attendi_il_via(&bandierine,numerogicatore);
    if (bandierina_presa(&bandierine,numerogicatore)){
        pausetta();
        if (sono_salvo(&bandierine,numerogicatore))
            printf("---->\tGIOCATORE %d HA PRESO LA BANDIERINA ED E' SALVO\n",numerogicatore +1 );
    } else {
        pausetta();
        if (ti_ho_preso(&bandierine,numerogicatore))
            printf("---->\tGIOCATORE %d HA PRESO L'ALTRO\n",numerogicatore +1);
    }

}

void attendi_giocatori( struct bandierine_t *b){
    printf("[ARBITRO]\t\tATTENDE GIOCATORI\n");
    sem_wait(&b->prontog1);
    sem_wait(&b->prontog2);
    pausetta();
}

void via (struct bandierine_t *b){
    printf("[ARBITRO]\t\tDO IL VIA\n\n");
    sem_post(&b->startg1);
    sem_post(&b->startg2);
}

int risultato_gioco (struct bandierine_t *b){
    if(b->vincitore == 2)
        return 2;
    else
        return 1;
}

void *arbitro ( void  *arg){
    attendi_giocatori(&bandierine);
    via(&bandierine);
    sem_wait(&bandierine.fine);
    printf("\n[ARBITRO]\t\tVINCE IL GIOCATORE\t%d\n", risultato_gioco(&bandierine) );
    return 0;
}



int main()
{
    int i=0;
    pthread_attr_t a;
    pthread_t pb;
    int *ris;

    /* inizializzo il mio sistema */
    init_bandierine(&bandierine);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(556);

    pthread_attr_init(&a);
    //pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    //creazione

    for (i=0; i<2; i++)
        pthread_create(&pb, &a, giocatore, (void *)(i));

    pthread_create(&pb, &a, arbitro, (void *)(2));

    // join
    for (i=0; i<3; i++)
        pthread_join(pb, (void **)ris);


    pthread_attr_destroy(&a);


    return 0;
}


