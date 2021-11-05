
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
int a,gio1,gio2;


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
    gio1 = gio2 =0;
    a=1;
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
        pthread_mutex_lock(&m);
        if(!a) pthread_cond_wait(&ar,&m);

        printf("premi un tasto per giocare...\n");
        scanf("%c",&via);
        pthread_cond_signal(&g1);
        pthread_cond_signal(&g1);
        // sleep(1);
        gio1 = gio2 = 1;
        a=0;
        printf("ehi mann\n");
        pthread_cond_wait(&g1,&m);
        pthread_cond_wait(&g2,&m);

        //pthread_cond_wait(&gio1,&m);
        //pthread_cond_wait(&gio2,&m);
        vincitore = decreta_vincitore(mossag1,mossag2);
        printf("sono qui.. vincitore %d\n",vincitore);
        switch (vincitore) {
            case 0:
                printf("il g1 ha tirato %s e g2 %s\t PAREGGIO\n",nomi_mosse[mossag1],nomi_mosse[mossag2]);
                break;
            case 1:
                printf("il g1 ha tirato %s e g2 %s\t VINCE G1\n",nomi_mosse[mossag1],nomi_mosse[mossag2]);
                break;
            case 2:
                printf("il g1 ha tirato %s e g2 %s\t VINCE G2\n",nomi_mosse[mossag1],nomi_mosse[mossag2]);
                break;
        }
        pthread_mutex_unlock(&m);
        //sem_wait(&sem_arbitro);
        //sem_wait(&sem_arbitro);
    }

}

void *giocatore1(void *arg)
{
    while(1) {
        //sleep(1);
        pthread_mutex_lock(&m);
        if(!gio1) pthread_cond_wait(&g1,&m);
        a=1;
        gio1=0;
        printf("ciao sono il giocatore 1\n");
        pthread_cond_wait(&g1, &m);
        printf("wait1 passata\n");
        mossag1 = rand() % 3;
        printf("Il giocatore1 ha effettuato la mossa \t %s\n", nomi_mosse[mossag1]);
        pthread_cond_signal(&ar);
        pthread_mutex_unlock(&m);

    }
}
void *giocatore2(void *arg) {
    while (1) {
        pthread_mutex_lock(&m);
        if (!gio2) pthread_cond_wait(&g1, &m);
        a = 1;
        gio2 = 0;
        printf("ciao sono il giocatore 2\n");
        pthread_cond_wait(&g2, &m);
        printf("wait1 passata\n");
        mossag1 = rand() % 3;
        printf("Il giocatore2 ha effettuato la mossa \t %s\n", nomi_mosse[mossag2]);
        pthread_cond_signal(&ar);
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
