#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

void main (){
   /* char s [5];
    int id = 3;
    sprintf(s,"%d\0",id);
    printf("questa e %s",s);*/

    int miovoto;
    printf("Vota con 0 oppure con 1\n");
    scanf("%d", &miovoto);
    printf("Votato %d\n",miovoto);

}