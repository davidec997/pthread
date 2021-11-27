#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define CARTAINA 0
#define TABACCO 2
#define FIAMMIFERI 2
char *nomi[3] = {"cartina", "tabacco", "fiammiferi"};
int tavolo [3];
sem_t sem_tabaccaio, sem_f1, sem_f2,sem_f3,m;
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
    for (int i = 0; i < 3; i++) tavolo[i]=0;
    sem_init(&sem_tabaccaio, 0, 1);
    sem_init(&sem_f1, 0, 0);
    sem_init(&sem_f2, 0, 0);
    sem_init(&sem_f3, 0, 0);

    sem_init(&m,0,1);

    fumatore1_ok = fumatore2_ok = fumatore3_ok = 0;
    tabaccaio_ok =1;
    turno=0;
}

void svegliaFumatore(){
    if (!tavolo[0]){
        // il primo elemento e' 0 quindi il tab ha messo sul tavolo gli altri 2
        //allora posso sevgliare il fumatore 0 che con gli ele 1 e 2 puo fumare
        printf("TABACCAIO SVEGLIA IL FUMATORE 1\n");
        sem_post(&m);
        sem_post(&sem_f1);
    }else if (!tavolo[1]){
        printf("TABACCAIO SVEGLIA IL FUMATORE 2\n");
        sem_post(&m);
        sem_post(&sem_f2);
    }else if (!tavolo[2]){
        printf("TABACCAIO SVEGLIA IL FUMATORE 3\n");
        sem_post(&m);
        sem_post(&sem_f3);
    }
    return;
}

void *tabaccaio(void *arg)
{
    //printf("Sono il tabaccaio e sto per dare mettere sul tavolo 2 elementi..\n");

    int id = (int) arg;
    int e1,e2;
    /*while(1){
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

            sem_wait(&m);
            vincitore = decreta_vincitore(mossag1, mossag2);
            printf("sono qui.. vincitore %d\n", vincitore);

            turno = 0;
            fumatore1_ok = fumatore2_ok = 0;
            sem_post(&m);
            pausetta();
        }
    }*/

    while(1){
        printf("Sono il tabaccaio e sto per dare mettere sul tavolo 2 elementi..\n");
        sem_wait(&sem_tabaccaio);
        sem_wait(&m);
        e1 = rand() % 3;
        e2 = rand() % 3;
        printf("Metto sul tavolo l'elemento %s e %s \n",nomi[e1],nomi[e2]);
        tavolo[e1]=1;
        tavolo[e2]=1;
        printf("\tSITUAZIONE [T] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        // facciamo che il tabaccaio sa quali ele ha messo sul tavolo e sveglia il fortunato che ha il terzo ele
        svegliaFumatore();
        sleep(5);
    }
}

void *fumatore1(void *arg)
{
    int id = (int) arg;
    printf("\tSONO IL FUMATORE %d e HO SOLO %s\n",id,nomi[id]);
    /*while(1) {
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
    }*/

    while(1){
        sem_wait(sem_f1);
        sem_wait(&m);
        //tolgo dal tavolo i 2 elementi
        printf("\tSITUAZIONE [f1 a] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        tavolo[1] = 0;
        tavolo[2] = 0;
        printf("\tSITUAZIONE [f1 b] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);

        printf("\t\tFUMATORE %d STA FUMANDO\n",id);
        sem_post(&m);
        //faccio la post al tabaccaio
        sem_post(&sem_tabaccaio);
        sleep(2);
    }

}

void *fumatore2(void *arg)
{
    int id = (int) arg;
    printf("\tSONO IL FUMATORE %d e HO SOLO %s\n",id,nomi[id]);
    /*while(1) {
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
    }*/

    while(1){
        sem_wait(sem_f2);
        sem_wait(&m);
        //tolgo dal tavolo i 2 elementi
        printf("\tSITUAZIONE [f2 a] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        tavolo[0] = 0;
        tavolo[2] = 0;
        printf("\t\tFUMATORE %d STA FUMANDO\n",id);
        printf("\tSITUAZIONE [f2 b] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        sem_post(&m);
        //faccio la post al tabaccaio
        sem_post(&sem_tabaccaio);
        sleep(2);
    }

}

void *fumatore3(void *arg)
{
    int id = (int) arg;
    printf("\tSONO IL FUMATORE %d e HO SOLO %s\n",id,nomi[id]);
    /*while(1) {
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
    }*/

    while(1){
        sem_wait(sem_f3);
        sem_wait(&m);
        //tolgo dal tavolo i 2 elementi
        printf("\tSITUAZIONE [f3 a] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        tavolo[0] = 0;
        tavolo[1] = 0;
        printf("\t\tFUMATORE %d STA FUMANDO\n",id);
        printf("\tSITUAZIONE [f3 b] TAVOLO [%d]  [%d]  [%d]\n",tavolo[0],tavolo[1],tavolo[2]);
        sem_post(&m);
        //faccio la post al tabaccaio
        sem_post(&sem_tabaccaio);
        sleep(2);
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
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, tabaccaio, 99);
    pthread_create(&p, &a, fumatore1, 0);
    pthread_create(&p, &a, fumatore2, 1);
    pthread_create(&p, &a, fumatore3, 2);


    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */\
    sleep(10);
    pthread_join(&p,NULL);


}

