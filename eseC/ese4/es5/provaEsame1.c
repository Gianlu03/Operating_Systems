/* FILE: pipe.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>
#define MSGSIZE 5 

int main (int argc, char **argv)
{   
    int pid; /* variabile per ritorno fork */
    int pidFiglio, status, ritorno; /* Variabili per wait*/
    int N = argc - 1; /* Variabile per numero parametri*/ 
    int piped[2][2]; /* piped[0] comunicazione pari, piped[1] per dispari*/
    int fd; /* Intero per contenere descrittore file*/
    char carLetto;

    /* Controllo numero parametri, devono essere esattamente 2*/
    if(N < 2){ /* parametri insufficienti */
        printf("Sono stati inseriti %d parametri, richiesti almeno 2\n", N);
        exit(1);
    }

    /*Creazione pipe*/
    for(int i = 0; i < 2; i++){
        if(pipe(piped[i]) < 0){
            printf("Errore apertura pipe %d", i);
            exit(2);
        }
    }
    
    for(int i = 0; i < N; i++){
        if((pid = fork()) < 0){
            /*Errore fork*/
            printf("Si è verificato un errore nella fork()\n");
            exit(2);
        }

        if(pid == 0){
            /*CODICE FIGLIO*/

            if((fd = open(argv[i+1], O_RDONLY)) < 0){
                /*Errore lettura nel file*/
                printf("Si è verificato un errore ad aprire in lettura %s", argv[i+1]);
                exit(-1);
            }
            printf("ho aperto %s\n", argv[i+1]);

            if((i+1) % 2 == 0){
                /*Caso file pari - voglio numeri*/
                close(piped[1][0]);
                close(piped[1][1]);
                close(piped[0][0]);

                while(read(fd, &carLetto, 1)){
                    if(isdigit(carLetto)){
                        printf("da %s scrivo %c\n", argv[i+1], carLetto);
                        write(piped[0][1], &carLetto, 1);
                    }
                }
            }
            else{
                /* Caso file dispari - voglio alfabetici*/
                close(piped[0][0]);
                close(piped[0][1]);
                close(piped[1][0]);

                while(read(fd, &carLetto, 1)){
                    if(isalpha(carLetto)){
                        //printf("da %s scrivo %c\n", argv[i+1], carLetto);
                        write(piped[1][1], &carLetto, 1);
                    }
                }
            }
            exit(carLetto);
        }
    }/* FINE FOR CREAZIONE FIGLI*/
    /* CODICE PADRE*/
    close(piped[0][1]); /* padre chiude lato scrittura su ambo le pipe*/
    close(piped[1][1]);

    sleep(3);
    printf("I caratteri letti sono:\n");
    sleep(3);
    int finitoPipe0 = 0, finitoPipe1 = 0;
    /* Ciclo che si interrompe solo se entrambe le pipe sono vuote*/
    for(;;){
        if(read(piped[1][0], &carLetto, 1) != 0)
            printf("%c", carLetto);
        else
            finitoPipe1 = 1;

        if(read(piped[0][0], &carLetto, 1) != 0)
            printf("%c", carLetto);
        else
            finitoPipe0 = 1;

        if(finitoPipe0 && finitoPipe1) /* Interrompo se concluse ambo le pipe*/
            break;
    }    
    printf("\n\n");

    /* CICLO ATTESA FIGLI*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){
            /*Errore wait*/
            printf("Errore wait\n");
            exit(3);
        }
        if((status & 0xFF) != 0){
            /*Terminazione anomala*/
            printf("Il figlio di pid %d è terminato in modo anomalo\n", pidFiglio);
        }
        else{
            ritorno=(int)((status >> 8) & 0xFF);
       		printf("Il figlio con pid=%d ha ritornato %c (se 255 problemi!)\n", pidFiglio, (char)ritorno);
        }
    }/* FINE FOR ATTESA FIGLI*/

	exit(0);
}