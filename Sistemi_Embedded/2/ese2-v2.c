
sem_t rst_priv, ab_priv, m;
Boolean is_rst;
Boolean is_ab;
int n_ab=0;
int b_rst=0;
int n_rst;
int b_ab;


int c_A,int c_B, c_Reset;          // conta le istanze in esecuzione
int b_B,int b_A, b_Reset;          // conta il numero dei bloccati

void startProcA(){
    sem_wait(&m);
    if(!n_rst && ! b_rst){
        n_ab ++;
        sem_post(&ab_priv);
    } else b_ab ++;

    sem_post(m);
    sem_wait(&ab_priv);
}

void startProcAbis(){
    sem_wait(&m);
    if(n_rst || b_rst){
        b_ab ++;
    } else {
        n_ab ++;
        sem_post(&ab_priv);
    };
    sem_post(&m);
    sem_wait(&ab_priv);
}

void endProcA(){
    sem_wait (&m);
    n_ab --;
    if (!n_ab && b_rst) {
        n_rst++;
        b_rst --;
        sem_post(rst_priv);
    }
    sem_post(&m);

}
void startProcRst(){
    sem_wait(&m);

    if(!n_ab){
        n_rst ++;
        sem_post(&rst_priv);
    } else b_rst ++;

    sem_post(&m);
    sem_wait(&rst_priv);

}

void endRst(){
    sem_wait(&m);
    n_rst --;
    if (b_rst){
        n_rst ++;
        b_rst --;
        sem_post(&rst_priv);
    } else if (b_ab) {
        while (b_ab) {
            n_ab++;
            b_ab --;
            sem_post(&priv_ab);
        }
    }

    sem_post_(&m);

}