//NON VA
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define CARTA 0
#define SASSO 1
#define FORBICE 2
char *nomi_mosse[3] = {"carta", "sasso", "forbice"};
sem_t sem_arbitro, sem_g1, sem_g2,m;
int mossag1,mossag2;
int arbitro_ok=1,giocatore1ok,giocatore2ok;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}
void myInit(void)
{
    sem_init(&sem_arbitro,0,1);
    sem_init(&sem_g1,0,0);
    sem_init(&sem_g2,0,0);
    sem_init(&m,0,1);

    mossag1 = mossag2 =-1;
    giocatore1ok = giocatore2ok = 0;
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

void *arbitro(void *arg)
{
    printf("Sono l'arbitro e sto per dare il via al gioco..\n");
    char via;
    int vincitore=-1;
    while(1){
        if(!arbitro_ok){
            sem_wait(&sem_arbitro);
        } else {
            printf("premi un tasto per giocare...\n");
            scanf("%c", &via);
            giocatore1ok = giocatore2ok = 1;
            sem_post(&sem_g1);
            sem_post(&sem_g2);
            printf("ho dato il via\n");
            if (!arbitro_ok){
                sem_wait(&sem_arbitro);
            } else {
                sem_wait(&sem_g1); //arbitr_sem
                sem_wait(&sem_g2);
                sem_wait(&m);
                vincitore = decreta_vincitore(mossag1, mossag2);
                printf("sono qui.. vincitore %d\n", vincitore);
                switch (vincitore) {
                    case 0:
                        printf("il g1 ha tirato %s e g2 %s\t PAREGGIO\n", nomi_mosse[mossag1], nomi_mosse[mossag2]);
                        break;
                    case 1:
                        printf("il g1 ha tirato %s e g2 %s\t VINCE G1\n", nomi_mosse[mossag1], nomi_mosse[mossag2]);
                        break;
                    case 2:
                        printf("il g1 ha tirato %s e g2 %s\t VINCE G2\n", nomi_mosse[mossag1], nomi_mosse[mossag2]);
                        break;
                }
                giocatore1ok =giocatore2ok = 0;
                sem_post(&m);
                pausetta();
                //sem_wait(&sem_arbitro);
                //sem_wait(&sem_arbitro);
            }
        }
}

}

void *giocatore1(void *arg)
{
    while(1) {
        //sleep(1);
        //sem_wait(&m);
        if (!giocatore1ok) {
            sem_wait(&sem_g1);
        } else {
            arbitro_ok = 0;
            printf("ciao sono il giocatore 1\n");
            //sem_wait(&sem_g1);
            printf("wait1 passata\n");
            mossag1 = rand() % 3;
            printf("Il giocatore1 ha effettuato la mossa \t %s\n", nomi_mosse[mossag1]);

            sem_post(&sem_g1);
            sem_post(&sem_arbitro);
            arbitro_ok = 1;
            //sem_post(&m);
        }
    }
}
void *giocatore2(void *arg)
{
    while(1) {
        if (!giocatore2ok) {
            sem_wait(&sem_g2);
        } else {
            arbitro_ok = 0;
            printf("ciao sono il giocatore 2\n");
            //sem_wait(&sem_g1);
            printf("wait2 passata\n");
            mossag1 = rand() % 3;
            printf("Il giocatore2 ha effettuato la mossa \t %s\n", nomi_mosse[mossag2]);

            sem_post(&sem_g2); //seve?
            sem_post(&sem_arbitro);
            arbitro_ok = 1;
            //sem_post(&m);
        }
    }
}


int main() {
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il sistema */
    myInit();

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    // pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, arbitro, 0);
    pthread_create(&p, &a, giocatore1, 1);
    pthread_create(&p, &a, giocatore2, 1);


    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */\
    sleep(10);
    pthread_join(&p,NULL);
    pthread_join(&p,NULL);
    pthread_join(&p,NULL);


}
