//STARVAAA
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

Esercizio 2

In un sistema organizzato secondo il modello a memoria comune viene
definita una risorsa astratta R sulla quale si puo` operare mediante
tre procedure identificate, rispettivamente, da ProcA, ProcB e Reset.

Le due procedure ProcA e ProcB, operano su variabili diverse della
risorsa R e pertanto possono essere eseguite concorrentemente tra loro
senza generare interferenze.

La procedura Reset opera su tutte le variabili di R e quindi deve
essere eseguita in modo mutuamente esclusivo sia con ProcA che con
ProcB.

1) Se i tre processi PA, PB e PR invocano, rispettivamernte, le
operazioni ProcA, ProcB e Reset, descrivere una tecnica che consente
ai processi PA e PB di eseguire le procedure da essi invocate senza
vincoli reciproci di mutua esclusione, garantendo invece la mutua
esclusione con l'esecuzione della procedura Reset invocata da PR.

Nel risolvere il problema garantire la priorita` alle esecuzioni di
Reset rispetto a quelle di ProcA e ProcB.

2) Qualora i processi che invocano le procedure ProcA e ProcB siano
piu` di due (PA1,......PAn e PB1,....,PBm) riscrivere la soluzione
garantendo anche la mutua esclusione tra due o piu` attivazioni di
ProcA e tra due o piu` attivazioni di ProcB.

*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

/*
Risoluzione del punto 1, con mutex e variabili condition o con semafori.

Il punto 2 è lasciato per esercizio :-)

Utilizzare l'apposita #define per scegliere l'implementazione...

(nell'implementazione con i mutex NON si è tenuto conto del problema della cancellazione dei threads).
*/

//#define USA_MUTEX
#define USA_SEM

/*
la struct R non è fornita dal testo; si sa solo che c'è e che va
utilizzata garantendo una certa concorrenza tra le varie attivazioni
delle funzioni ProcA, ProcB e Reset.

Per questo motivo, occorre fornire solo i tre processi PA, PB e PR.
Le procedure ProcA, ProcB e Reset in questa implementazione di esempio
non prendono parametri, in realtà potrebbero avere un puntatore ad una
struttura di tipo R.
*/

/*
   Nota bene:
   c_Reset vale sempre o 0 o 1!!!
*/



/* ------------------------------------------------------------------------ */

/* risoluzione con mutex e variabili condition */

#ifdef USA_MUTEX

pthread_mutex_t m;
pthread_cond_t priv_AB;
pthread_cond_t priv_Reset;
int c_AB, c_Reset;          // conta le istanze in esecuzione
int b_AB, b_Reset;          // conta il numero dei bloccati

void myInit(void)
{
  pthread_mutexattr_t m_attr;
  pthread_condattr_t c_attr;

  pthread_mutexattr_init(&m_attr);
  pthread_condattr_init(&c_attr);

  pthread_mutex_init(&m, &m_attr);
  pthread_cond_init(&priv_AB, &c_attr);
  pthread_cond_init(&priv_Reset, &c_attr);

  pthread_condattr_destroy(&c_attr);
  pthread_mutexattr_destroy(&m_attr);

  c_AB = c_Reset = b_AB = b_Reset = 0;
}

void StartProcA(void)
{
  pthread_mutex_lock(&m);
  while (c_Reset || b_Reset) {
    b_AB++;
    pthread_cond_wait(&priv_AB, &m);
    b_AB--;
  }
  c_AB++;
  pthread_mutex_unlock(&m);
}

void EndProcA(void)
{
  pthread_mutex_lock(&m);

  c_AB--;
  if (b_Reset && !c_AB)
    pthread_cond_signal(&priv_Reset);

  pthread_mutex_unlock(&m);
}


// le procedure di B si comportano in modo identico a quelle di A
void StartProcB(void)
{
  StartProcA();
}

void EndProcB(void)
{
  EndProcA();
}

void StartReset(void)
{
  pthread_mutex_lock(&m);
  while (c_AB) {
    b_Reset++;
    pthread_cond_wait(&priv_Reset, &m);
    b_Reset--;
  }
  c_Reset++;
  pthread_mutex_unlock(&m);
}

void EndReset(void)
{
  pthread_mutex_lock(&m);

  c_Reset--;
  if (b_AB)
    pthread_cond_broadcast(&priv_AB);

  pthread_mutex_unlock(&m);
}

#endif

/* ------------------------------------------------------------------------ */

/* risoluzione con semafori */

#ifdef USA_SEM

sem_t m;
sem_t priv_A, priv_B;
sem_t priv_Reset;
int c_A,c_B, c_Reset;          // conta le istanze in esecuzione
int b_A,b_B, b_Reset;          // conta il numero dei bloccati

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&priv_A,0,0);
    sem_init(&priv_A,0,0);

    sem_init(&priv_Reset,0,0);

    c_A = c_Reset = b_A = b_Reset = c_B = b_B = 0;
}

void StartProcA(void)
{
    sem_wait(&m);

    if (c_Reset >0 || b_Reset > 0 || c_A > 0) {
        b_A++;
    }
    else {
        c_A++;
        sem_post(&priv_A);
    }
    sem_post(&m);
    sem_wait(&priv_A);
}

void EndProcA(void)
{
    sem_wait(&m);

    c_A--;
    if (b_Reset > 0 && c_A == 0 && c_B == 0) {
        c_Reset++;
        b_Reset--;
        sem_post(&priv_Reset);
    }  else if (b_A > 0 && c_A == 0){
        b_A --;
        c_A ++;
        sem_post(&priv_A);
    }
    sem_post(&m);
}

void StartProcB(void)
{
    sem_wait(&m);

    if (c_Reset > 0 || b_Reset > 0 || c_B > 0 ) {
        b_B++;
    }
    else {
        c_B++;
        sem_post(&priv_B);
    }
    sem_post(&m);
    sem_wait(&priv_B);
}

void EndProcB(void)
{
    sem_wait(&m);

    c_B--;
    if (b_Reset > 0 && c_A == 0 && c_B == 0) {
        c_Reset++;
        b_Reset--;
        sem_post(&priv_Reset);
    }else if (b_B > 0 && c_B == 0){
        b_B--;
        c_B ++;
        sem_post(&priv_B);
    }

    sem_post(&m);
}

void StartReset(void)
{
    sem_wait(&m);
    if (c_A > 0 || c_B > 0 ) {
        b_Reset++;
    }
    else {
        c_Reset++;
        sem_post(&priv_Reset);
    }
    sem_post(&m);
    sem_wait(&priv_Reset);
}

void EndReset(void)
{
    sem_wait(&m);
    c_Reset--;
    if (c_Reset == 0) {
        if(b_Reset > 0) {
            c_Reset++;
            b_Reset--;
            sem_post(&priv_Reset);
        } else{
            if (b_A > 0) {
                c_A++;
                b_A--;
                sem_post(&priv_A);
            }
            if (b_B > 0) {
                c_B++;
                b_B--;
                sem_post(&priv_B);
            }
        }
    }
        sem_post(&m);

}
#endif

/* ------------------------------------------------------------------------ */

/* alla fine di ogni ciclo ogni thread aspetta un po'.
   Cosa succede se tolgo questa nanosleep?
   di fatto solo i thread di tipo B riescono ad entrare --> starvation!!!!
   (provare per credere)
*/
void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

/* ------------------------------------------------------------------------ */

/* le funzioni della risorsa R fittizia */

#define BUSY 1000000
#define CYCLE 50

void myprint(char *s, int id)
{
    int i,j;
    fprintf(stderr,"[");
    for (j=0; j<CYCLE; j++) {
        fprintf(stderr,s,id);
        for (i=0; i<BUSY; i++);
    }
    fprintf(stderr,"]");
}
void ProcA(int id)
{
    printf("THREAD A CON ID %lu\n",id);
    myprint("%d",id);
}

void ProcB(int id)
{
    printf("THREAD B CON ID %lu\n",id);
    myprint("%d",id);
}

void Reset(int id)
{
    printf("THREAD R CON ID %lu\n",id);
    myprint("%d",id);
}


void *PA(void *arg)
{
    int id = (int) arg;
    printf(" CREATO THREAD A CON ID %lu\n",id);

    for (;;) {
        fprintf(stderr,"A");
        StartProcA();
        ProcA(id);
        EndProcA();
        fprintf(stderr,"a");
        sleep(1);
    }
    return 0;
}

void *PB(void *arg)
{
    int id = (int)arg;
    printf(" CREATO THREAD B CON ID %lu\n",id);

    for (;;) {
        fprintf(stderr,"B");
        StartProcB();
        ProcB(id);
        EndProcB();
        fprintf(stderr,"b");

        sleep(1);
    }
    return 0;
}

void *PR(void *arg)
{
    int id = (int)id;
    printf("CREATO THREAD R CON ID %lu\n",id);

    for (;;) {
        fprintf(stderr,"R");
        StartReset();
        Reset(id);
        EndReset();
        fprintf(stderr,"r");

    }
    return 0;
}

/* ------------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il sistema */
    myInit();

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);


    pthread_create(&p, &a, PA, 1);
    pthread_create(&p, &a, PA, 2);
    //pthread_create(&p, &a, PA, 2);
    //pthread_create(&p, &a, PB, 3);
    pthread_create(&p, &a, PB, 5);
    pthread_create(&p, &a, PB, 6);

    pthread_create(&p, &a, PR, 0);

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(5);

    return 0;
}
