/*
 * Copyright (C) 2004 by Paolo Gai
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
Soluzione esame del 9 giugno 2005
*/


// usare semafori o variabili condition?
//#define USA_MUTEX
#define USA_SEM

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define N 5

#ifdef USA_SEM

/* la struttura condivisa */
struct codaclienti_t {
    sem_t mutex;

    int attesa[N];
    sem_t priv[N];

    int clientecorrente;
    sem_t priv_pizzaiolo;
    int c_p;
} coda;

void init_codaclienti(struct codaclienti_t *in)
{
    int i;

    sem_init(&in->mutex,0,1);
    for (i=0; i<N; i++) {
        sem_init(&in->priv[i],0,0);
        in->attesa[i] = 0;
    }

    in->clientecorrente = -1;
    sem_init(&in->priv_pizzaiolo,0,0);
    in->c_p = 0;
}

void ordina_pizze(struct codaclienti_t *codaclienti,
                  int numeropizze, int numerocliente)
{
    sem_wait(&codaclienti->mutex);

    codaclienti->attesa[numerocliente] = numeropizze;

    printf("[cl %d, p %d]\n", numerocliente, numeropizze);

    if (codaclienti->c_p) {
        codaclienti->clientecorrente = numerocliente;
        codaclienti->c_p--;
        sem_post(&codaclienti->priv_pizzaiolo);
    }
    else
        sem_post(&codaclienti->mutex);
}


void ritira_pizze(struct codaclienti_t *codaclienti, int numerocliente)
{
    sem_wait(&codaclienti->priv[numerocliente]);

    printf("[ritiro cl %d]\n", numerocliente);
}

void prossima_pizza(struct codaclienti_t *codaclienti)
{
    int min, i, cliente;

    sem_wait(&codaclienti->mutex);

    printf("(servo:");
    for (i=0; i<N; i++) printf(" %d", codaclienti->attesa[i]);
    printf(")\n");

    if (codaclienti->clientecorrente == -1) {
        // devo considerare l'ordine pendente + corto
        // ovvero il minimo numero maggiore di 0

        min = 0;
        cliente = -1;
        for (i=0; i<N; i++)
            if (codaclienti->attesa[i]) {
                min = codaclienti->attesa[i];
                cliente = i;
                break;
            }

        while (i<N) {
            if (codaclienti->attesa[i] && min > codaclienti->attesa[i]) {
                min = codaclienti->attesa[i];
                cliente = i;
            }
            i++;
        }

        // se non ci sono ordini devo bloccarmi
        if (!min) {
            codaclienti->c_p++;
            sem_post(&codaclienti->mutex);
            sem_wait(&codaclienti->priv_pizzaiolo); // passaggio del testimone
        }
        else
            codaclienti->clientecorrente = cliente;
    }

    printf("(servo cl %d, p %d)\n",
           codaclienti->clientecorrente,
           codaclienti->attesa[codaclienti->clientecorrente]);

    sem_post(&codaclienti->mutex);
}

void consegna_pizza(struct codaclienti_t *codaclienti)
{
    sem_wait(&codaclienti->mutex);

    // qui sicuramente ho un ordine pendente da evadere
    codaclienti->attesa[codaclienti->clientecorrente]--;

    printf("(servito cl %d, rimanenti %d)\n",
           codaclienti->clientecorrente,
           codaclienti->attesa[codaclienti->clientecorrente]);

    if (!codaclienti->attesa[codaclienti->clientecorrente]) {
        sem_post(&codaclienti->priv[codaclienti->clientecorrente]);
        codaclienti->clientecorrente = -1;
    }

    sem_post(&codaclienti->mutex);
}

#endif

/* ------------------------------ */


#ifdef USA_MUTEX

/* la struttura condivisa */
struct codaclienti_t {
  pthread_mutex_t mutex;

  int attesa[N];
  pthread_cond_t priv[N];

  int clientecorrente;
  pthread_cond_t priv_pizzaiolo;
  int c_p;
} coda;


void init_codaclienti(struct codaclienti_t *in)
{
  int i;
  pthread_mutexattr_t m_attr;
  pthread_condattr_t c_attr;

  pthread_mutexattr_init(&m_attr);
  pthread_condattr_init(&c_attr);

  pthread_mutex_init(&in->mutex, &m_attr);

  for (i=0; i<N; i++) {
    pthread_cond_init(&in->priv[i], &c_attr);
    in->attesa[i] = 0;
  }

  in->clientecorrente = -1;
  pthread_cond_init(&in->priv_pizzaiolo, &c_attr);
  in->c_p = 0;

  pthread_condattr_destroy(&c_attr);
  pthread_mutexattr_destroy(&m_attr);
}



void ordina_pizze(struct codaclienti_t *codaclienti,
		  int numeropizze, int numerocliente)
{
  pthread_mutex_lock(&codaclienti->mutex);

  codaclienti->attesa[numerocliente] = numeropizze;

  printf("[cl %d, p %d]\n", numerocliente, numeropizze);

  if (codaclienti->c_p) {
    //    codaclienti->clientecorrente = numerocliente;
    //    codaclienti->c_p--;
    pthread_cond_signal(&codaclienti->priv_pizzaiolo);
  }
  pthread_mutex_unlock(&codaclienti->mutex);
}


void ritira_pizze(struct codaclienti_t *codaclienti, int numerocliente)
{
  pthread_mutex_lock(&codaclienti->mutex);

  printf("[vado per ritirare cl %d]\n", numerocliente);

  // non ho usato un contatore dei bloccati, in quanto il pizzaiolo fa
  // comunque una signal (considero difficile il caso in cui il
  // cliente non debba attendere :-)
  while (codaclienti->attesa[numerocliente])
    pthread_cond_wait(&codaclienti->priv[numerocliente], &codaclienti->mutex);

  printf("[ritiro cl %d]\n", numerocliente);

  pthread_mutex_unlock(&codaclienti->mutex);
}

void prossima_pizza(struct codaclienti_t *codaclienti)
{
  int min, i, cliente;

  pthread_mutex_lock(&codaclienti->mutex);

  printf("(servo:");
  for (i=0; i<N; i++) printf(" %d", codaclienti->attesa[i]);
  printf(")\n");

  while (codaclienti->clientecorrente == -1) {
    // devo considerare l'ordine pendente + corto
    // ovvero il minimo numero maggiore di 0

    min = 0;
    cliente = -1;
    for (i=0; i<N; i++)
      if (codaclienti->attesa[i]) {
	min = codaclienti->attesa[i];
	cliente = i;
	break;
      }

    while (i<N) {
      if (codaclienti->attesa[i] && min > codaclienti->attesa[i]) {
	min = codaclienti->attesa[i];
	cliente = i;
      }
      i++;
    }

    // se non ci sono ordini devo bloccarmi
    if (!min) {
      codaclienti->c_p++;
      pthread_cond_wait(&codaclienti->priv_pizzaiolo, &codaclienti->mutex);
      codaclienti->c_p--;
    }
    else
      codaclienti->clientecorrente = cliente;
  }

  printf("(servo cl %d, p %d)\n",
	 codaclienti->clientecorrente,
	 codaclienti->attesa[codaclienti->clientecorrente]);

  pthread_mutex_unlock(&codaclienti->mutex);
}

void consegna_pizza(struct codaclienti_t *codaclienti)
{
  pthread_mutex_lock(&codaclienti->mutex);

  // qui sicuramente ho un ordine pendente da evadere
  codaclienti->attesa[codaclienti->clientecorrente]--;

  printf("(servito cl %d, rimanenti %d)\n",
	 codaclienti->clientecorrente,
	 codaclienti->attesa[codaclienti->clientecorrente]);

  if (!codaclienti->attesa[codaclienti->clientecorrente]) {
    pthread_cond_signal(&codaclienti->priv[codaclienti->clientecorrente]);
    codaclienti->clientecorrente = -1;
  }

  pthread_mutex_unlock(&codaclienti->mutex);
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
    int numeropizze;

    while (1) {
        numeropizze = rand()%10+1;
        pausetta(100000);
        ordina_pizze(&coda, numeropizze, numerocliente);
        pausetta(100000);
        ritira_pizze(&coda, numerocliente);
        pausetta(100000);
    }
    return 0;
}

void *pizzaiolo(void *arg)
{
    while (1) {
        prossima_pizza(&coda);
        pausetta(0);
        consegna_pizza(&coda);
    }
    return 0;
}


/* la creazione dei thread */

int main()
{
    int i=0;
    pthread_attr_t a;
    pthread_t pa, pb;

    /* inizializzo il mio sistema */
    init_codaclienti(&coda);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (i=0; i<N; i++)
        pthread_create(&pb, &a, cliente, (void *)(i));

    pthread_create(&pa, &a, pizzaiolo, NULL);

    pthread_attr_destroy(&a);

    sleep(5);

    return 0;
}


