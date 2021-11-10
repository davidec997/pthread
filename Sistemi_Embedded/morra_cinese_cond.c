
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
pthread_mutex_t m;
pthread_cond_t g1,g2,ar;
int mossag1,mossag2;
int a,giocatore1ok,giocatore2ok;
int  turno;


void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}
void myInit(void)
{
    pthread_cond_init(&ar,NULL);
    pthread_cond_init(&g1,NULL);
    pthread_cond_init(&g2,NULL);
    pthread_mutex_init(&m,NULL);

    mossag1 = mossag2 =-1;
    giocatore1ok = giocatore2ok =0;
    a=1;
    turno = 0;
}

int decreta_vincitore(int g1, int g2){
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
    int t = 0;
    int vincitore=-1;
    int *pi = (int *) arg;
    while(1){
        pthread_mutex_lock(&m);
        while(turno != t){
            pthread_cond_wait(&ar,&m);
            if (turno == 1) pthread_cond_signal(&g1);
            if (turno == 2) pthread_cond_signal(&g2);
        }
            //pthread_mutex_lock(&m);
            printf("premi un tasto per giocare...\n");
            scanf("%c", &via);
            turno = 1;
            pthread_cond_signal(&g1);
            //pthread_cond_signal(&g2);
           // printf("ho dato il via\n");
            pthread_mutex_unlock(&m);
            while (turno != t){
                pthread_cond_wait(&ar,&m);
                if (turno == 1) pthread_cond_signal(&g1);
                if (turno == 2) pthread_cond_signal(&g2);
            }
            //pthread_mutex_lock(&m);
            vincitore = decreta_vincitore(mossag1, mossag2);
            //printf("sto per decretare il vincitore..  %d\n", vincitore);
            switch (vincitore) {
                case 0:
                    printf("il g1 ha tirato %s e g2 %s\t PAREGGIO\n", nomi_mosse[mossag1], nomi_mosse[mossag2]);
                    break;
                case 1:
                    printf("il g1 ha tirato %s e g2 %s\t VINCE GIOCATORE 1\n", nomi_mosse[mossag1], nomi_mosse[mossag2]);
                    break;
                case 2:
                    printf("il g1 ha tirato %s e g2 %s\t VINCE GIOCATORE 2\n", nomi_mosse[mossag1], nomi_mosse[mossag2]);
                    break;
            }
            turno = 0;
            giocatore1ok = giocatore2ok = 0;
             pthread_mutex_unlock(&m);
            pausetta();
        }

}

void *th_votante(void *arg)
{
    //int *pi = (int *) arg;
    int t = 1;
    while(1) {
        while (turno != t) {
            //printf("il gio1 si e blocc\n");
            pthread_cond_wait(&g1,&m);
            if (turno == 0) pthread_cond_signal(&ar);
            if (turno == 2) pthread_cond_signal(&g2);
        }
        //printf("ciao sono il giocatore 1\n");
        mossag1 = rand() % 3;
        printf("Il giocatore1 ha effettuato la mossa \t %s\n", nomi_mosse[mossag1]);
        giocatore1ok = 1;
        turno = 2;
        pthread_cond_signal(&g2);
        /*if (giocatore1ok && giocatore2ok) {
            turno = 0;
            pthread_cond_signal(&ar);
        }*/
        pthread_mutex_unlock(&m);
        //pthread_yield();
    }
}

void *giocatore2(void *arg)
{
    //int *pi = (int *) arg;
    int t = 2;
    while(1) {
        while (turno != t ) {
            //printf("il gio2 si e blocc\n");
            pthread_cond_wait(&g2,&m);
            if (turno == 0) pthread_cond_signal(&ar);
            if (turno == 2) pthread_cond_signal(&g2);
        }
       // printf("ciao sono il giocatore 2\n");
        mossag2 = rand() % 3;
        printf("Il giocatore2 ha effettuato la mossa \t %s\n", nomi_mosse[mossag2]);
        giocatore2ok = 1;
        if (giocatore1ok && giocatore2ok) {
            turno = 0;
            pthread_cond_signal(&ar);
        }
        pthread_mutex_unlock(&m);
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
