#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<string.h>
#include<errno.h>
#include<time.h>
#define PERM 644

int main(){
    int pidFiglio, status;  /* variabili di ritorno di wait */

    printf("Padre: pid = %d\n", getpid());

    if((pidFiglio = wait(&status)) < 0){
        printf("Errore wait: figlio non presente\n\n");
        exit(errno);
    }
    
    printf("Figlio atteso correttamente\n\n");



    exit(0);
}