#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>


int main(){

    char **riparazioni;
    riparazioni = malloc(4 * 20);
    riparazioni[0] = "carrozziere\0";
    riparazioni[1] = "meccanico\0";
    riparazioni[1] = "pompista\0";
    riparazioni[1] = "elettrauto\0";

    int *array;
    srand(time(NULL));

    array = malloc(10 * sizeof (int ));
    for (int i = 0; i < 10; i++) {
        array[i] = rand() % 10;
        printf("\t%d\t",array[i]);
    }

    //printf("%s",riparazioni[0]);
    int ord = 0;
    int c = 0;
    int min_ele = array[0];
    /*for (int i = 0; i < NUM_THREADS-1; i++) {
        printf("\t\t\t%d\t",p->ordini[i]);
        if (p->ordini[i] < p->ordini[ord] && p->ordini[i] > 0)
            ord = i;
    }*/

    for ( int i = 0; i< 10; i++ ){
        //printf("\t%d\t",array[i]);

        if ( array[i] < min_ele ){
            min_ele = array[i];
            ord = i;
        }
    }

    printf("\n MIN ELE %d AT INDEX %d\n",array[ord], ord);

}