#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

int trovaCassiere (int * arr ){
    int index = -1;
    int min = 999;
    for (int i = 0; i < 10; i++) {
        if (arr[i] < min){
            index = i;
            min = arr[i];
        }


    }

    if (index == -1) printf("\tERRORR\n");
    return  index;
}


int *myarr;
int N;
sem_t  semaforo;

int main (int argc, char **argv) {

    sem_init(&semaforo,0,5);
    int val;
    sem_getvalue(&semaforo,&val);

    printf("VAL DEL SEMAFORO %d\n",val);



/*
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = (rand()% 40) + 1;
        printf("\t%d\t",arr[i]);
    }

    int index = trovaCassiere(arr);
    printf("\nCassiere con meno cosi %d\t\t%d\n",index, arr[index]);*/


}