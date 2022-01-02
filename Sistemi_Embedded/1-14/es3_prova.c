//
// Created by davide on 02/01/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define N 8

sem_t full;
sem_t empty;
sem_t mutex;

typedef struct busta_t{
    char mess;
    int prior;
}busta;

int tail;
int head;
busta mailbox[N];

void init(){
    sem_init(&mutex,0,1);
    sem_init(&full,0,N);
    sem_init(&empty,0,0);
    tail=head=0;
    for(int i=0;i<N;++i){
        mailbox[i].prior=3;
    }
    tail=head=0;
}

void inserisciInMailbox(busta b){
    int index=0;
    for(int i=tail;i<tail+N;++i){
        if(b.prior < mailbox[i%N].prior &&  mailbox[i%N].prior !=3)
            index=i;
    }
    int tmp_head=head;
    int x,x_1;
    for(int i=index;i<=(N+head)%N;++i){
        x=(tmp_head+N)%N;
        x_1 = ((tmp_head+N-1)%N);
        mailbox[x].mess = mailbox[x_1].mess;
        mailbox[x].prior = mailbox[x_1].prior;
        tmp_head=x_1;
    }
    mailbox[tmp_head].mess = b.mess;
    mailbox[tmp_head].prior = b.prior;
}

void send(int arg){
    sem_wait(&full);
    sem_wait(&mutex);
    int p = rand()%3; //scelgo una priorità
    busta b;
    b.mess = 'a';
    b.prior = p;
    inserisciInMailbox(b);
    printf("Sono il mittente %lu e ho scritto nella busta\n",pthread_self());
    head = (head+1)%N;
    sem_post(&mutex);
    sem_post(&empty);
}

void receive(int arg){
    sem_wait(&empty);
    sem_wait(&mutex);
    busta b = mailbox[tail];
    tail = (tail+1)%N;
    printf("Sono il destinatario %lu e ho letto\n",pthread_self());
    sem_post(&mutex);
    sem_post(&full);
}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*90000000000;
    nanosleep(&t,NULL);
}


void *mittente(void *arg){
    for(;;){
        send(*(int *)arg);
        sleep(2);
    }

}

void *ricevente(void *arg){
    for(;;){
        receive(*(int*)arg);
        sleep(2);
    }

}


int main(int argc, char **argv)
{
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il sistema */
    init();

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
    for(int i=0; i <5;++i){
        pthread_create(&p, &a, mittente, (void *)&i);
        pthread_create(&p, &a, ricevente, (void *)&i);
    }

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(5);

    return 0;
}




