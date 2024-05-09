/* FILE: pipe.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include<string.h>
#include <ctype.h>
#define MSGSIZE 5 

typedef int pipe_t[2];

int main (int argc, char **argv)
{   
    int fd; /* Variabile per file descriptor del singolo file*/
    int pid; /* Variabile per ritorno fork()*/
    int pidFiglio, status, ritorno; /* Variabili per Wait */
    int N = argc-2; /* N = numero caratteri*/
    pipe_t *piped;
    char* usage = "USAGE: ./main filename Cf1 Cf2 ...\n"; /* Stringa da stampare in caso di errore dell'utente*/

    /* Controllo numero parametri lasco: almeno 3 parametri*/
    if(N + 1 < 3){
        printf("Numero parametri errato: necessari almeno 3 parametri, inseriti %d\n", N+1);
        printf("%s", usage);
        exit(1);
    }

    /* Controllo che i successivi parametri */
    for(int i = 2; i < argc; i++){
        if(strlen(argv[i]) != 1){
            /* Un parametro non è carattere singolo*/
            printf("Parametro %s non è un carattere\n", argv[i]);
            printf("%s", usage);
            exit(3);
        }
    }

    /*Il file viene controllato dai figli, in questo modo non si usa lo stesso fdi*/

    printf("DEBUG: numero parametri corretto\n\n");

    piped = (pipe_t *) malloc (N*sizeof(pipe_t));
    if (piped == NULL)
    { 
        printf("Errore nella allocazione della memoria\n");
        exit(3); 
    }


    /* Creo una pipe per ogni file (N) */
    for(int i = 0; i < N; i++){
        if(pipe(piped[i]) < 0){
            printf("Errore creazione piped[%d]\n", i);
            exit(4);
        }
    }

    /*CREAZIONE FIGLI*/
    for(int i = 0; i < N; i++){
        if((pid = fork()) < 0){
            printf("Errore nella fork\n");
            exit(5);
        }

        if(pid == 0){
            /*CODICE FIGLIO*/
            
            if((fd = open(argv[1], O_RDONLY)) < 0){
                printf("Impossibile aprire il file %s, controllare esistenza o diritti\n", argv[1]);
                printf("%s", usage);
                exit(2);
            }

            /*Chiusura pipe*/
            for (int k=0; k < N; k++)
            {   /*chiuse tutte le pipe in lettura*/
                close(piped[k][0]);
                if (k != i)
                    close(piped[k][1]); /* Ogni figlio chiude tutti i lati lettura tranne il proprio*/
            }

            long int contaOccorrenze = 0; /*Variabile per contare occorrenze*/
            char carLetto; /*Variabile buffer per contenere carattere letto*/

            /*Leggo contenuto file carattere per carattere*/
            while(read(fd, &carLetto, 1)){
                if(carLetto == argv[i+2][0]) /*Se il carattere coincide con il parametro aumenta conteggio*/
                    contaOccorrenze++;
            }
            
            /*Scrivo su Pipe conteggio per inviarlo al padre*/
            //printf("DEBUG:Scrivo sulla pipe %d il conteggio %ld\n", i, contaOccorrenze);
            write(piped[i][1], &contaOccorrenze, sizeof(contaOccorrenze));

            /*Una volta finito il figlio ritorna il proprio carattere*/
            exit(argv[i+2][0]);
        }
    }

    /*CODICE PADRE*/
    for (int k=0; k < N; k++)
    {   /*Padre chiude tutte le pipe in scrittura*/
        close(piped[k][1]);
    }

    /*Stampa caratteri e occorenze trovate*/
    int index = 0; /*indice */
    long int conteggio;
    while(index < N && read(piped[index][0], &conteggio, sizeof(conteggio))){ /* Leggo il conteggio e lo associo al carattere di posizione index+2*/
        printf("carattere %c: %ld occorrenze\n", argv[index+2][0], conteggio);
        index++;
    }

    /*ATTESA FIGLI*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){
            printf("Errore Wait\n");
            exit(6);
        }

        if((status & 0xFF) != 0)
            printf("Processo di pid = %d terminato in modo anomalo\n", pidFiglio);
        else{
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo di pid = %d ha ritornato il carattere %c\n", pidFiglio, (char)ritorno);
        }
    }

    exit(0);
}