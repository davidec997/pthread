
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

//5 filosofi
#define N 5
#define DELAY 20000000
//struttura per la gestione dello stato e filosofi
int filosofi = N;
int stato [N];
typedef  enum {false, true} Boolean;
Boolean forchette [5];

// def sequenze
#define THINKING 0
#define HUNGRY 1
#define EATING 2

//semafori
sem_t m;
sem_t priv_f [N];


void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}
void myInit(void)
{
    sem_init(&m,0,1);
    for (int i = 0; i <N; i++) sem_init(&priv_f[i],0,1);
    for (int i = 0; i <N; i++) forchette[i] = false;
    for(int t =0;t<N;t++) stato[t] = THINKING;
}

void think(int index){
    sem_wait(&m);
    printf("Sono il filosofo %d e sto PENSANDO\n",index);
    sem_post(&m);
    for (int t = 0; t< DELAY; t ++);
     printf("Ho finito di pensare %d\n",index);
}

int check(int index){
    return ((stato[index] == HUNGRY) &&(stato[(index + 4) % N] != EATING) && (stato[(index +1) % N] != EATING));
}

void preEat(int index){
    printf("sono in preeat%d\n",index);

    sem_wait(&m);
    stato[index] = HUNGRY;
    sem_wait(&priv_f[index]);

    if (check(index)) {
        stato[index]= EATING;
        if (index == 4) {
            //il quarto prende prima la forchetta alla sinistra e poi dx
           forchette[index] = false;
           forchette[(index + 4) % N] = false;
        }  else {
            forchette[(index + 4) % N] = false;
            forchette[index] = false;
        }
        printf("Il filosofo %d ha ottenuto le bacchette [%d] e [%d] \n", index,index ,(index + 4) % N);
    }

    sem_post(&m);
}

void postEat(int index){
    printf("Sono in postEat %d\n",index);
    sem_wait(&m);
    //rimetto a posto le postate
    printf("Sono il filosofo %d e ho FINITO di mangiare.\n",index);

    forchette[(index)% N] = true;
    forchette[(index + 4)% N] = true;

    stato[index] = THINKING;
    //cerco di svegliare i miei vicini se hanno fame
    if(check((index + 1) % N)){
        sem_post(&priv_f[(index + 1) % N]);
        stato[(index + 1) % N] = EATING;
    } else if (check((index + 4) % N)){
        sem_post(&priv_f[(index + 4) % N]);
        stato [(index + 4) % N] = EATING;
    } else {
        sem_post(&priv_f[index % N]);
    }

    sem_post(&m);
    printf("Ho finito la post eat%d\n",index);

}

void eat(int index){

    sem_post(&m);
    printf("Situazione FORCHETTE attuale :  [%d] [%d] [%d] [%d] [%d]\n",forchette[0], forchette[1],forchette[2], forchette[3],forchette[4]);
    /*for(int i =0; i< N; i++){
        if(i == 4) {
            printf("%d [%d]\n",index, forchette[i]);
        } else {
            printf("%d [%d]\t", index, forchette[i]);
        }
    }*/
    //printf("Sono il filosofo %d e sto INIZIANDO a mangiare con le bacchette [%d] e [%d]\n", index,((index ) % N),((index + 4) % N));
    for (int i = 0; i < DELAY; ++i);
    printf("Sono il filosofo %d e non voglio piu  mangiare...\n", index);

    sem_post(&m);

}

void *dinner(void *arg)
{
    int index = (int)arg;
    for (int f =0;f<20;f++) {
        //printf("Ciaooo sono il filo %d e che esegue un nuovo ciclo \n",index);
       think(index);
       preEat(index);
       eat(index);
       postEat(index);
    }
    return 0;
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

    pthread_create(&p, &a, dinner, 0);
    pthread_create(&p, &a, dinner, 1);
    pthread_create(&p, &a, dinner, 2);
    pthread_create(&p, &a, dinner, 3);
    pthread_create(&p, &a, dinner, 4);

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(10);

}
