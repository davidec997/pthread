#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
char *nomi_mosse[3] = {"carta", "sasso", "forbice"};
#define CARTA 0
#define SASSO 1
#define FORBICE 2

pthread_mutex_t mutex;
    sem_t piena, vuota;

//array circolare
void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

struct CircularArray_t {
    int array[10];
    int head, tail, num;
} queue;

void init_CA(struct CircularArray_t *a)
{ a->head=0; a->tail=0; a->num=0; }


int insert_CA(struct CircularArray_t *a, int elem)
{ if (a->num == 10) return 0;
    a->array[a->head] = elem;
    a->head = (a->head + 1) % 10;
    a->num++;
    return 1;
}
int extract_CA(struct CircularArray_t *a,
               int *elem)
{ if (a->num == 0) return 0;
    *elem = a->array[a->tail];
    a->tail = (a->tail + 1) % 10;
    a->num--;
    return 1;
}

void *Primo ( void * arg ) {
    int ins;
    for(int i = 0; i< 20;i++){
        sem_wait(&vuota);
        pthread_mutex_lock(&mutex);
        ins = rand()%10;
        insert_CA(&queue,ins);
        printf("Ho inserito il val %d nella posizione %d\n",ins,queue.head);
        sem_post(&piena);
        pthread_mutex_unlock(&mutex); /* rilascio mutua esclusione, cosi' Secondo parte */
        pausetta();
    }
}

void *Secondo ( void *arg ) {
    int ret;
    for(int i = 0; i< 20;i++) {
        sem_wait(&piena);
        pthread_mutex_lock(&mutex);
        extract_CA(&queue,&ret);
        printf("Ho estratto %d dalla posizione %d\n",ret,queue.tail);
        sem_post(&vuota);
        pthread_mutex_unlock(&mutex); /* rilascio mutua esclusione, così Primo può partire */
        pausetta();

    }
}

int main ()
{ pthread_t th;
    sleep(1);
    init_CA(&queue);
    pthread_mutex_init(&mutex, NULL);
    sem_init(&piena,0,0);
    sem_init(&vuota,0,10);


    pthread_create( &th, NULL, Primo, NULL);
    pthread_create( &th, NULL, Secondo, NULL);
    pthread_join(&th,NULL);
    pthread_join(&th,NULL);

    pthread_exit( NULL );
}
/*

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


int main (){
    srand(555);
    int g1,g2,vincitore;
    for(int i=0;i<10;i++){
        g1= rand()%3;
        printf("g1 %s\n",nomi_mosse[g1]);
        g2= rand()%3;
        printf("g1 %s\n",nomi_mosse[g2]);
        vincitore= decreta_vincitore(g1,g2);
        switch (vincitore) {
            case 0:
                printf("il g1 ha tirato %s e g2 %s\t PAREGGIO\n",nomi_mosse[g1],nomi_mosse[g2]);
                break;
            case 1:
                printf("il g1 ha tirato %s e g2 %s\t VINCE G1\n",nomi_mosse[g1],nomi_mosse[g2]);
                break;
            case 2:
                printf("il g1 ha tirato %s e g2 %s\t VINCE G2\n",nomi_mosse[g1],nomi_mosse[g2]);
                break;
        }
    }

}*/
