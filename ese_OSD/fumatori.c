#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define CARTAINA 0
#define TABACCO 1
#define FIAMMIFERI 2
char *nomi[3] = {"cartina", "tabacco", "fiammiferi"};
sem_t sem_tabaccaio, sem_f1, sem_f2,sem_f3,m;
int mossag1,mossag2;
int tabaccaio_ok,fumatore1_ok,fumatore2_ok,fumatore3_ok,turno;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}
void myInit(void)
{
    sem_init(&sem_tabaccaio, 0, 1);
    sem_init(&sem_f1, 0, 0);
    sem_init(&sem_f2, 0, 0);
    sem_init(&sem_f3, 0, 0);

    sem_init(&m,0,1);

    //mossag1 = mossag2 =-1;
    fumatore1_ok = fumatore2_ok = fumatore3_ok = 0;
    tabaccaio_ok =1;
    turno=0;
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

void *tabaccaio(void *arg)
{
    printf("Sono il tabaccaio e sto per dare mettere sul tavolo 2 elementi..\n");

    int id = (int) arg;
    while(1){
        if(id != turno){
            sem_wait(&sem_tabaccaio);
        } else {
            sem_wait(&m);

            turno = 1;
            sem_post(&sem_f1);
            sem_post(&sem_f2);
            printf("ho dato il via\n");
            sem_post(&m);
            if (turno != id){
                sem_wait(&sem_tabaccaio);
            }
            //sem_wait(&sem_f1); //arbitr_sem
            //sem_wait(&sem_f2);
            sem_wait(&m);
            vincitore = decreta_vincitore(mossag1, mossag2);
            printf("sono qui.. vincitore %d\n", vincitore);

            turno = 0;
            fumatore1_ok = fumatore2_ok = 0;
            sem_post(&m);
            pausetta();
        }

    }

}

void *genreratore_O(void *arg)
{
    int id = (int) id;
    while(1) {
        if (turno != id) {
            printf("il gio1 si e blocc\n");
            sem_wait(&sem_f1); // aspetto l tabaccaio
        }
        sem_wait(&m);
        printf("ciao sono il giocatore 1\n");
        mossag1 = rand() % 3;
        printf("Il fumatore1 ha effettuato la mossa \t %s\n", nomi[mossag1]);
        fumatore1_ok = 1;
        if (fumatore1_ok && fumatore2_ok) {
            turno = 0;
            sem_post(&sem_tabaccaio);
        }
        sem_post(&m);

    }
}
void *fumatore2(void *arg)
{
    int id = (int) arg;
    while(1) {
        if (turno != id) {
            printf("il gio2 si e blocc\n");
            sem_wait(&sem_f2); // aspetto l tabaccaio
        }
        sem_wait(&m);
        printf("ciao sono il giocatore 1\n");
        mossag2 = rand() % 3;
        printf("Il fumatore2 ha effettuato la mossa \t %s\n", nomi[mossag2]);
        fumatore2_ok = 1;
        if (fumatore1_ok && fumatore2_ok) {
            turno = 0;
            sem_post(&sem_tabaccaio);
        }
        sem_post(&m);
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

    pthread_create(&p, &a, tabaccaio, 0);
    pthread_create(&p, &a, fumatore1, 1);
    pthread_create(&p, &a, fumatore2, 1);
    pthread_create(&p, &a, fumatore3, 1);


    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */\
    sleep(10);
    pthread_join(&p,NULL);


}

