/*
 * Nessuna modifica al funzionamento o alla logica apportata.
 * L'unica miglioria apportata e':
 * sposto la sem_post(&b->fine) dal 'body' del thread giocatore alle funzioni sono_salvo e ti_ho_preso in quanto rileggendo\
 * con piu' calma il testo ho notato che l'arbitro deve stampare il vincitore appena possibile.
 *
 * aggiunte varie stampe di controllo e pausette()
 * */

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

/*
 * la variabile bandierina_presa conterra' il numero del giocatore che ha preso la bandierina
 *
 * 'salvo' e 'preso' vengono usati come flag: se ad esempio il giocatore 1 prende la bandierina e quando entra nella funzione\
 * sono_salvo trova salvo settata ancora a 0 --> allora setta 'salvo' a 1 e invalida 'preso' ( valore -1) --> vince g1
 *
 * viceversa se il giocatore 2 quando entra nella funzione ti_ho_preso trova 'preso' settata a 0 vuol dire che g1 ha preso\
 * la bandierina ma non e' ancora entrato in 'sono_salvo' --> quindi setta 'preso' a 1 e invalida salvo (valore -1) --> vince g2
 *
 * la variabile vincitore contiene il numero del giocatore che ha vinto
 * */


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
    if (ngio){          //G2
        sem_post(&b->prontog2);
        sem_wait(&b->startg2);
        printf("\t\t\tGIOCATORE 2 PARTE\n");
    } else{             //G1
        sem_post(&b->prontog1);
        sem_wait(&b->startg1);
        printf("\t\t\tGIOCATORE 1 PARTE\n");
    }
}

int bandierina_presa (struct bandierine_t *b, int ngio){
    int ris = 0;
    pausetta();

    sem_wait(&b->m);
    if (ngio){          // se ngio == 1 --> sono il giocatore 2
        if (b->bandierina_presa == 0){
            b->bandierina_presa = 2;
            ris = 1;
            printf("[GIOCATORE %d]\tPRENDE LA BANDIERINA\n",ngio +1);   // ngio + 1 per maggior chiarezza dell'output
        }
    } else {            // G1
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
    pausetta();

    sem_wait(&b->m);
    if (ngio){      //G2
        if (b->salvo == 0){
            b->salvo = 1;
            b->preso = -1;
            ris = 1;
            b->vincitore = 2;
            printf("[GIOCATORE %d]\tPRENDE LA BANDIERINA ED E' SALVO!\n",ngio +1);
            sem_post(&b->fine);             // <-- aggiunta per quanto detto all'inizio
        }
    } else {           //G1
        if (b->salvo == 0){
            b->salvo = 1;
            b->preso = -1;
            ris = 1;
            b->vincitore = 1;
            printf("[GIOCATORE %d]\tPRENDE LA BANDIERINA ED E' SALVO!\n",ngio +1);
            sem_post(&b->fine);           // <-- aggiunta
        }
    }

    sem_post(&b->m);
    return ris;
}

int ti_ho_preso (struct bandierine_t *b, int ngio){
    int ris = 0;
    pausetta();

    sem_wait(&b->m);
    if (ngio){
        if (b->preso == 0){
            b->salvo = -1;
            b->preso = 1;
            ris = 1;
            b->vincitore = 2;
            printf("[GIOCATORE %d]\tNON HA PRESO LA BANDIERINA MA HA RAGGIUNTO L'ATRO!!\n",ngio +1);
            sem_post(&b->fine);         // <-- aggiunta
        }
    } else {
        if (b->preso == 0){
            b->salvo = -1;
            b->preso = 1;
            ris = 1;
            b->vincitore = 1;
            printf("[GIOCATORE %d]\tNON HA PRESO LA BANDIERINA MA HA RAGGIUNTO L'ATRO!!\n",ngio +1);
            sem_post(&b->fine);         // <-- aggiunta
        }
    }

    sem_post(&b->m);
    return ris;
}

void *giocatore ( void * arg){
    int numerogicatore = (int) arg;
    attendi_il_via(&bandierine,numerogicatore);
    if (bandierina_presa(&bandierine,numerogicatore)){
        if (sono_salvo(&bandierine,numerogicatore))
            printf("---->\tGIOCATORE %d HA PRESO LA BANDIERINA ED E' SALVO\n",numerogicatore +1 );
    } else {
        if (ti_ho_preso(&bandierine,numerogicatore))
            printf("---->\tGIOCATORE %d HA PRESO L'ALTRO\n",numerogicatore +1);
    }
    //    sem_post(&bandierine.fine);    // spostata in sono_salvo e ti_ho_preso

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
    //sem_wait(&bandierine.fine);   // non necessaria perche' solo un thread fara' la post su fine appena il risultato e' certo

    printf("\n[ARBITRO]\t\tVINCE IL GIOCATORE\t%d\n", risultato_gioco(&bandierine));
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
    srand(time(NULL));

    pthread_attr_init(&a);

    //creazione
    // giocatori con id 0 e 1
    for (i=0; i<2; i++)
        pthread_create(&pb, &a, giocatore, (void *)(i));

    //arbitro con id 2
    pthread_create(&pb, &a, arbitro, (void *)(2));

    // join
    for (i=0; i<3; i++)
        pthread_join(pb, (void **)ris);


    pthread_attr_destroy(&a);


    return 0;
}


