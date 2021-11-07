sem_t m;
sem_t priv_A;
sem_t priv_B;
sem_t priv_Reset;
int c_A,int c_B, c_Reset;          // conta le istanze in esecuzione
int b_B,int b_A, b_Reset;          // conta il numero dei bloccati

void myInit(void)
{
    sem_init(&m,0,1);
    sem_init(&priv_A,0,0);
    sem_init(&priv_B,0,0);
    sem_init(&priv_Reset,0,0);

    c_A = c_B = c_Reset = b_B = b_A = b_Reset = 0;
}

void StartProcA(void)
{
    sem_wait(&m);
    if (c_A || c_Reset || b_Reset){
        //mi blocco segno che c e una A blocc
        b_A++;
    } else {
        sem_post (&priv_A);
        c_A++;
    }
    sem_post(&m);
    sem_wait(&priv_A);
}

void EndProcA(void)
{
    sem_wait(&m);

    c_A--;
    if ( !c_A && b_Reset && !c_AB) {
        c_Reset++;
        b_Reset--;
        sem_post(&priv_Reset);
    } else if (!b_Reset && !c_Reset && b_A){
        b_A --;
        c_A ++;
        sem_post(&priv_A);
    }

    sem_post(&m);
}


// le procedure di B si comportano in modo identico a quelle di A
void StartProcB(void)
{
    sem_wait(&m);
    if (c_B || c_Reset || b_Reset){
        //mi blocco segno che c e una B blocc
        b_B++;
    } else {
        sem_post (&priv_B);
        c_B++;
    }
    sem_post(&m);
    sem_wait(&priv_B);}

void EndProcB(void)
{
    sem_wait(&m);

    c_B--;
    if ( !c_B && b_Reset && !c_AB) {
        c_Reset++;
        b_Reset--;
        sem_post(&priv_Reset);
    } else if (!b_Reset && !c_Reset && b_B){
        b_B --;
        c_B ++;
        sem_post(&priv_B);
    }

    sem_post(&m);}

void StartReset(void)
{
    sem_wait(&m);
    if (c_A || c_B) {
        b_Reset++;
    }
    else {
        sem_post(&priv_Reset);
        c_Reset++;
    }
    sem_post(&m);
    sem_wait(&priv_Reset);
}

void EndReset(void)
{
    sem_wait(&m);

    c_Reset--;
    if (b_A) {
        sem_post(&priv_A);
        b_A--;
        c_A++;
    } else if(b_B){
        sem_post(&priv_B);
        b_B--;
        c_B++;
    }

    sem_post(&m);
}