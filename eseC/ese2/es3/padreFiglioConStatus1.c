#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<string.h>
#include<errno.h>
#include<time.h>

#define PERM 666

int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    return casuale;
}


int main(){
    srand(time(NULL));

    int pid, pidAtteso, random;
    int status, ritorno;  /* variabili per stato ritornato da wait */

    printf("Padre con pid = %d\n", getpid());

    if((pid = fork()) < 0){
        /*  Errore nella fork  */
        perror("Errore fork");
        exit(errno);
    }

    if(pid == 0){
        /* codice figlio */
        printf("Figlio: ho pid = %d e mio padre ha ppid = %d", getpid(), getppid());
        random = mia_random(100); //genero numero casuale con funzione fornita
        printf(" -> ho generato %d\n", random);
        exit(random);
    }

    /*ELSE: codice padre*/

    if((pidAtteso = wait(&status)) < 0){
        /* ERRORE WAIT */
        perror("Errore WAIT");
        exit(errno);
    }

    if((status & 0xFF) != 0){
        /* FIGLIO TERMINATO IN MODO ANOMALO */
        printf("Figlio di pid = %d terminato in modo anomalo\n", pidAtteso);
    }
    else{
        /* FIGLIO TERMINATO CORRETTAMENTE */
        status = status >> 8;
        ritorno = (int)(status &= 0xFF);  // tolgo bit "sporchi"
        printf("PADRE: pid figlio = %d -> figlio ha generato %d\n", pidAtteso, ritorno);
    }

    printf("\nPROVO AD ATTENDERE UN ALTRO FIGLIO:\n");
    /* EFFETTUO WAIT PER SECONDO FIGLIO INESISTENTE */
    if((wait(&status)) < 0){
        /* ERRORE WAIT */
        perror("Errore WAIT");
        exit(errno);
    }


    exit(0);
}