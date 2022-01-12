/*
 * Copyright (C) 2005 by Paolo Gai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
Soluzione esame del 19 settembre 2005
*/


// usare semafori o variabili condition?

//#define USA_MUTEX
//define USA_SEM
#define USA_SEM_FAC


// numero di clienti
#define C 15
// numero di cassieri
#define N 3

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef USA_SEM

/* la struttura condivisa */
struct supermercato_t {
  sem_t mutex;

  sem_t scassieri[N];
  int ccassieri[N];
  int totaleoggetti[N];

  // i clienti si accodano sulle casse
  sem_t sclienti[C];
  int cclienti[C];
 
} supermercato;

void init_supermercato(struct supermercato_t *s)
{
  int i;

  /* mutua esclusione */
  sem_init(&s->mutex,0,1);
  for (i=0; i<N; i++) {
    sem_init(&s->scassieri[i],0,0);
    s->ccassieri[i] = 0;
    s->totaleoggetti[i] = 0;
  }

  for (i=0; i<C; i++) {
    sem_init(&s->sclienti[i],0,0);
    s->cclienti[i] = 0;
  }
}

void cliente_entra_in_coda(struct supermercato_t *s, 
			   int numerocliente, 
			   int numerooggetti)
{
  int minimo,cassa,i;
  
  sem_wait(&s->mutex);

  /* cerco la cassa con il minor numero i oggetti */
  minimo = s->totaleoggetti[0];
  cassa = 0;
  for (i=1; i<N; i++)  // si suppone N>=1
    if (minimo > s->totaleoggetti[i]) {
      minimo = s->totaleoggetti[i];
      cassa = i;
    }

  /* conto il numero di oggetti*/
  s->totaleoggetti[cassa] += numerooggetti;

  printf("cliente %2d: oggetti %2d ARRIVO (cassa %d con %d oggetti)\n",
	 numerocliente, 
	 numerooggetti, 
	 cassa, 
	 s->totaleoggetti[cassa]);

  /* a questo punto devo bloccarmi passando il testimone */
  s->cclienti[cassa]++;
  if (s->ccassieri[cassa]) {
    sem_post(&s->scassieri[cassa]);
  }
  else
    sem_post(&s->mutex);
  sem_wait(&s->sclienti[cassa]);
  s->cclienti[cassa]--;

  /* quando mi risveglio tolgo il numero di oggetti e me ne vado */
  s->totaleoggetti[cassa] -= numerooggetti;

  printf("cliente %2d: oggetti %2d         ESCO (cassa %d con %d oggetti)\n",
	 numerocliente, 
	 numerooggetti,
	 cassa, 
	 s->totaleoggetti[cassa]);

  sem_post(&s->mutex);
}

void cassiere_servo_cliente(struct supermercato_t *s, int numerocassa)
{
  sem_wait(&s->mutex);
  
  /* mi blocco quando non c'e' nessun cliente in coda sulla mia cassa */
  if (!s->cclienti[numerocassa]) {
    // la cassa � vuota, mi blocco
    s->ccassieri[numerocassa]++;
    sem_post(&s->mutex);
    sem_wait(&s->scassieri[numerocassa]);
    s->ccassieri[numerocassa]--;
  }

  sem_post(&s->mutex);
}

void cassiere_fine_cliente(struct supermercato_t *s, int numerocassa)
{
  sem_wait(&s->mutex);
  // passo di sicuro il testimone al cliente che deve uscire
  sem_post(&s->sclienti[numerocassa]);
}

#endif







/* ------------------------------ */









#ifdef USA_SEM_FAC

/* questa struttura contiene informazioni relative ad un cliente in
   coda presso un cassiere */
struct cliente_t {
    int noggetti;
    int numerocliente;
};

/* la coda pu� al massimo essere lunga C clienti (spreco memoria ma
   sono sicuro :-) */
struct coda_t {
    int totaleoggetti;
    int head;
    int tail;
    int n;
    struct cliente_t clienti[C];
};

/* la struttura condivisa */
struct supermercato_t {
    sem_t mutex;

    sem_t scassieri[N];
    int ccassieri[N];
    struct coda_t code[N];

    // nota: un semaforo per ogni cliente!
    sem_t sclienti[C];
    int cclienti[C];
} supermercato;

void init_supermercato(struct supermercato_t *s)
{
    int i,j;

    /* mutua esclusione */
    sem_init(&s->mutex,0,1);
    for (i=0; i<N; i++) {
        sem_init(&s->scassieri[i],0,0);
        s->ccassieri[i] = 0;

        s->code[i].totaleoggetti = 0;
        s->code[i].head = 0;
        s->code[i].tail = 0;
        s->code[i].n = 0;
        for (j=0; j<C; j++) {
            s->code[i].clienti[j].noggetti = 0;
            s->code[i].clienti[j].numerocliente = 0;
        }
    }

    for (j=0; j<C; j++) {
        sem_init(&s->sclienti[i],0,0);
        s->cclienti[i] = 0;
    }
}

void cliente_entra_in_coda(struct supermercato_t *s,
                           int numerocliente,
                           int numerooggetti)
{
    int minimo,cassa,i;

    sem_wait(&s->mutex);

    /* cerco la cassa con il minor numero i oggetti */
    minimo = s->code[0].totaleoggetti;
    cassa = 0;
    for (i=1; i<N; i++)  // si suppone N>=1
        if (minimo > s->code[i].totaleoggetti) {
            minimo = s->code[i].totaleoggetti;
            cassa = i;
        }

    /* conto il numero di oggetti*/
    s->code[cassa].totaleoggetti += numerooggetti;

    /* mi accodo */
    s->code[cassa].n++;
    s->code[cassa].clienti[s->code[cassa].head].noggetti = numerooggetti;
    s->code[cassa].clienti[s->code[cassa].head].numerocliente = numerocliente;
    s->code[cassa].head = (s->code[cassa].head+1)%C;

    printf("cliente %2d: oggetti %2d ARRIVO     (cassa %d con %d client, %d oggetti)\n",
           numerocliente,
           numerooggetti,
           cassa,
           s->code[cassa].n,
           s->code[cassa].totaleoggetti);

    if (s->code[cassa].n==1 && s->ccassieri[cassa]) {
        /* sono il primo e il cassiere mi sta aspettando. sveglio il cassiere */
        sem_post(&s->scassieri[cassa]);
        s->ccassieri[cassa]--;
    }

    /* mi blocco sempre */
    s->cclienti[numerocliente]++;

    sem_post(&s->mutex);
    sem_wait(&s->sclienti[numerocliente]);
}

void cassiere_servo_cliente(struct supermercato_t *s, int numerocassa)
{
    sem_wait(&s->mutex);

    /* mi blocco quando non c'e' nessun cliente in coda sulla mia cassa */
    if (s->code[numerocassa].n) {
        // la coda presso la cassa non � vuota, posso continuare
        sem_post(&s->scassieri[numerocassa]);
    }
    else {
        // non c'e' nessuno, mi blocco
        s->ccassieri[numerocassa]++;
    }

    sem_post(&s->mutex);
    sem_wait(&s->scassieri[numerocassa]);
}

void cassiere_fine_cliente(struct supermercato_t *s, int numerocassa)
{
    int elem;
    sem_wait(&s->mutex);

    // tolgo il cliente dalla coda
    s->code[numerocassa].n--;
    elem = s->code[numerocassa].tail;
    s->code[numerocassa].tail = (s->code[numerocassa].tail+1)%C;

    // aggiorno il numero di oggetti totale
    s->code[numerocassa].totaleoggetti -= s->code[numerocassa].clienti[elem].noggetti;

    // sveglio il cliente
    sem_post(&s->sclienti[s->code[numerocassa].clienti[elem].numerocliente]);

    printf("cliente %2d: oggetti %2d       ESCO (cassa %d con %d client, %d oggetti)\n",
           s->code[numerocassa].clienti[elem].numerocliente,
           s->code[numerocassa].clienti[elem].noggetti,
           numerocassa,
           s->code[numerocassa].n,
           s->code[numerocassa].totaleoggetti);


    sem_post(&s->mutex);
}

#endif








/* ------------------------------ */









#ifdef USA_MUTEX

/* la struttura condivisa */
struct supermercato_t {
  pthread_mutex_t mutex;

  pthread_cond_t scassieri[N];
  int ccassieri[N];
  int totaleoggetti[N];

  // i clienti si accodano sulle casse
  pthread_cond_t sclienti[C];
  int cclienti[C];
 
} supermercato;

void init_supermercato(struct supermercato_t *s)
{
  int i;

  pthread_mutexattr_t m_attr;
  pthread_condattr_t c_attr;

  pthread_mutexattr_init(&m_attr);
  pthread_condattr_init(&c_attr);

  pthread_mutex_init(&s->mutex, &m_attr);
  for (i=0; i<N; i++) {
    pthread_cond_init(&s->scassieri[i], &c_attr);
    s->ccassieri[i] = 0;
    s->totaleoggetti[i] = 0;
  }

  for (i=0; i<C; i++) {
    pthread_cond_init(&s->sclienti[i], &c_attr);
    s->cclienti[i] = 0;
  }

  pthread_condattr_destroy(&c_attr);
  pthread_mutexattr_destroy(&m_attr);
}

void cliente_entra_in_coda(struct supermercato_t *s, 
			   int numerocliente, 
			   int numerooggetti)
{
  int minimo,cassa,i;
  
  pthread_mutex_lock(&s->mutex);

  /* cerco la cassa con il minor numero i oggetti */
  minimo = s->totaleoggetti[0];
  cassa = 0;
  for (i=1; i<N; i++)  // si suppone N>=1 
    if (minimo > s->totaleoggetti[i]) {
      minimo = s->totaleoggetti[i];
      cassa = i;
    }

  /* conto il numero di oggetti*/
  s->totaleoggetti[cassa] += numerooggetti;

  printf("cliente %2d: oggetti %2d ARRIVO (cassa %d con %d oggetti)\n",
	 numerocliente, 
	 numerooggetti, 
	 cassa, 
	 s->totaleoggetti[cassa]);

  /* a questo punto devo bloccarmi passando il testimone. Dal momento
     che sulla variabile condition c'e' solo un cliente, non serve il
     while */
  s->cclienti[cassa]++;
  if (s->ccassieri[cassa])
    pthread_cond_signal(&s->scassieri[cassa]);
  pthread_cond_wait(&s->sclienti[cassa], &s->mutex);
  s->cclienti[cassa]--;

  /* quando mi risveglio tolgo il numero di oggetti e me ne vado */
  s->totaleoggetti[cassa] -= numerooggetti;

  printf("cliente %2d: oggetti %2d         ESCO (cassa %d con %d oggetti)\n",
	 numerocliente, 
	 numerooggetti,
	 cassa, 
	 s->totaleoggetti[cassa]);

  pthread_mutex_unlock(&s->mutex);
}

void cassiere_servo_cliente(struct supermercato_t *s, int numerocassa)
{
  pthread_mutex_lock(&s->mutex);
  
  /* mi blocco quando non c'e' nessun cliente in coda sulla mia cassa */
  if (!s->cclienti[numerocassa]) {
    // la cassa � vuota, mi blocco
    s->ccassieri[numerocassa]++;
    pthread_cond_wait(&s->scassieri[numerocassa], &s->mutex);
    s->ccassieri[numerocassa]--;
  }

  pthread_mutex_unlock(&s->mutex);
}

void cassiere_fine_cliente(struct supermercato_t *s, int numerocassa)
{
  pthread_cond_signal(&s->sclienti[numerocassa]);
}




#endif


/* ------------------------------ */


void pausetta(int quanto)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%100+1)*1000000 + quanto;
    nanosleep(&t,NULL);
}

void *cliente(void *arg)
{
    int numerocliente = (int)arg;
    int numerooggetti;
    while (1) {
        pausetta(100000);
        numerooggetti=rand()%10;
        cliente_entra_in_coda(&supermercato, numerocliente, numerooggetti);
    }
    return 0;
}

void *cassiere(void *arg)
{
    int numerocassa = (int)arg;
    while(1) {
        pausetta(100000);
        printf("                                    cassiere %d: ARRIVO\n", numerocassa);
        cassiere_servo_cliente(&supermercato, numerocassa);
        printf("                                    cassiere %d:         SERVO\n", numerocassa);
        pausetta(100000);
        cassiere_fine_cliente(&supermercato, numerocassa);
        printf("                                    cassiere %d:                FINITO\n", numerocassa);
        pausetta(100000);
    }
}

/* la creazione dei thread */

int main()
{
    int i=0;
    pthread_attr_t a;
    pthread_t pb;

    /* inizializzo il mio sistema */
    init_supermercato(&supermercato);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(556);

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (i=0; i<N; i++)
        pthread_create(&pb, &a, cassiere, (void *)(i));

    for (i=0; i<C; i++)
        pthread_create(&pb, &a, cliente, (void *)(i));

    pthread_attr_destroy(&a);

    sleep(40);

    return 0;
}


