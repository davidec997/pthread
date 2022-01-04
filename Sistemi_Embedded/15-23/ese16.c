
/*
 *  16  Supponiamo che nella facolta` vi sia un solo locale destinato ai servizi igienici, costituito da 5 toilette.
 *  Il locale Ë condiviso da tutti gli studenti, uomini e donne, con la regola che in ogni istante l'intero locale
puÚ essere utilizzato escusivamente o da donne o da uomini.
a)Utilizzando il meccanismo semaforico, simulare quanto necessario per sincronizzare gli utenti del
locale servizi. (nel descrivere la soluzione non preoccuparsi di individuare particolari strategie di
allocazione o di evitare la starvation).
b)Riscrivere la soluzione implementando una strategia di accesso che elimini problemi di starvation.
 * */

// simile a lettori scrittori

//PERFETTO
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define N 5
#define NTIMES 10
typedef enum {false,true} Boolean;
int maschi_in_bagno, maschi_attesa, femmine_in_bagno, femmine_attesa;
sem_t s_maschi, s_femmine,m,femmine_stop,maschi_stop;
int posti;
int max_f, max_m;

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&s_maschi,0,0);
    sem_init(&s_femmine,0,0);
    sem_init(&femmine_stop,0,0);
    sem_init(&maschi_stop,0,0);

    posti = N;
    maschi_attesa = maschi_in_bagno = femmine_attesa = femmine_in_bagno = 0;
    max_f = max_m = N;
}

void controllaAccesso (int *pi, Boolean female){
    sem_wait(&m);
    printf("[SERVICE]\t\tATTUALMENTE CI SONO %d MASCHI in bagno\t %d MASCHI in attesa\t%d FEMMINE in bagno\t %d FEMMINE in attesa\n",maschi_in_bagno,maschi_attesa,femmine_in_bagno,femmine_attesa);

    if(female){
        // se sono donna controllo che ci siano posti e che non ci siano uomini in bagno
        if(posti > 0 && maschi_in_bagno == 0 && max_f > 0){
            //posso entrare
            posti --;
            femmine_in_bagno ++;
            sem_post(&s_femmine);
            max_f --;
        } else
            femmine_attesa ++;
        sem_post(&m);
        sem_wait(&s_femmine);

    } else { // sono maschio
        if (posti > 0 && femmine_in_bagno == 0 && max_m > 0){
            posti --;
            maschi_in_bagno ++;
            max_m --;
            sem_post(&s_maschi);
        } else
            maschi_attesa ++;
        sem_post(&m);
        sem_wait(&s_maschi);
    }
}

void esciDalBagno (int *pi , Boolean female){
    sem_wait(&m);

    posti ++;

    if (female) {
        femmine_in_bagno--;     // qui voglio fare che se sono gia entrate 5 fem (max_f == 0) faccio partire un maschio
        if (femmine_in_bagno == 0 && max_f == 0 && maschi_attesa > 0) {
            printf("\t\t\t\t\t\t\t ultima donna fa partire un uomo\n");
            max_m = N - 1;
            maschi_attesa--;
            maschi_in_bagno++;
            sem_post(&s_maschi);        // se sono gia entrate 5 femmine.. faccio entrare un maschio x evitare starvation
        }

        if ((femmine_in_bagno > 0 || maschi_in_bagno == 0) && femmine_attesa > 0 && max_f>0) {
            printf("\t\t\t\t\t\t\t femmina e' qui siiiiiiiiiiiii\n");
            femmine_attesa--;
            posti--;
            femmine_in_bagno++;
            max_f--;
            sem_post(&s_femmine);
        }
    }
    else{ // maschio
        maschi_in_bagno --;
        if (maschi_in_bagno == 0 && max_m == 0 && femmine_attesa > 0){      // discorso di prima invertito
            printf("\t\t\t\t\t\t\t ultimo uomo fa partire una donna\n");
            max_f = N -1;
            femmine_attesa --;
            femmine_in_bagno ++;
            sem_post(&s_femmine);
        }
        if (maschi_in_bagno > 0 && maschi_attesa > 0 && max_m >0) {
            printf("\t\t\t\t\t\t\t maschio e' qui nooooooooooo\n");
            posti--;
            maschi_in_bagno++;
            maschi_attesa--;
            max_m--;
            sem_post(&s_maschi);
        }
    }

    printf("[SERVICE]\t\tATTUALMENTE CI SONO %d MASCHI in bagno\t %d MASCHI in attesa\t  %d FEMMINE in bagno\t %d FEMMINE in attesa\n",maschi_in_bagno,maschi_attesa,femmine_in_bagno,femmine_attesa);

    sem_post(&m);
}

void *eseguiUtente(void *id) {
    int *pi = (int *) id;
    int *ptr;
    ptr = (int *) malloc(sizeof(int));
    Boolean female = (*pi % 2);
    int volte_in_bagno = 0;

    if (ptr == NULL) {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
    }
    if (female){
        printf("[UTENTE %d]\t SONO UNA FEMMINA. --> %d\n",*pi,female);
    } else {
        printf("[UTENTE %d]\t SONO UN MASCHIO. --> %d\n",*pi,female);

    }
    sleep(rand()%13);

    for(int t =0 ;t<NTIMES;t++){
        controllaAccesso(pi, female);
        if (female)
            printf("[UTENTE %d DONNA]\t SONO ENTRATA IN BAGNO. --> %d\n",*pi);
        else
            printf("[UTENTE %d UOMO]\t SONO ENTRATO IN BAGNO. --> %d\n",*pi);

        //faccio le mie cose
        sleep(1);
        volte_in_bagno ++;
        esciDalBagno(pi,female);
        if (female)
            printf("[UTENTE %d DONNA]\t SONO USCITA DAL BAGNO. --> %d\n",*pi);
        else
            printf("[UTENTE %d UOMO]\t SONO USCITO DAL BAGNO. --> %d\n",*pi);
        sleep(3);

    }

    *ptr = volte_in_bagno;
    pthread_exit((void *) ptr);
}


int main (int argc, char **argv)
{
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    int NUM_THREADS;
    char error[250];

    /* Controllo sul numero di parametri */
    if (argc != 2 ) /* Deve essere passato esattamente un parametro */
    {
        sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
    }

    /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0)
    {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
    }

    myInit();
    srand(555);
    thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
    {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
    }
    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        if (pthread_create(&thread[i], NULL, eseguiUtente, (void *) (&taskids[i])) != 0)
        {
            sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI DELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
    }

    for (i=0; i < NUM_THREADS; i++)
    {
        int ris;
        /* attendiamo la terminazione di tutti i thread generati */
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    pthread_mutex_destroy(&m);
    exit(0);
}



