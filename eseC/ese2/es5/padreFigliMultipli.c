/* FILE COPIATO DA: status1.c */
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<string.h>
#include<errno.h>
#include<time.h>

int main (int argc, char* argv[])
{
    /* controllo numero parametri */
    if(argc != 2)
    {   /* num parametri errato, deve essere esattamente 1 */
        printf("Non è stato inserito esattamente 1 parametro!\n");
        printf("USAGE: ./main interoPositivo\n");
        exit(1);
    }
    /*numero parametri corretto, proseguo*/
    int N; //variabile per contenere parametro
    int status, pidFiglio; /* variabili per valori di ritorno wait */

    if(atoi(argv[1]) <= 0 && atoi(argv[1]) < 255)
    { /*parametro errato, non è intero positivo*/
        printf("Non è stato inserito un parametro intero positivo!\n");
        printf("USAGE: ./main interoPositivo\n");
        exit(2);
    }
    N = atoi(argv[1]);

    printf("DEBUG: parametro corretto\n\n");

    /* stampo pid e N come richiesto */
    printf("PADRE: pid = %d, N = %d\n", getpid(), N);

    int i;
    for(i = 0; i < N; i++)
    {   /* ESSENDO IL CODICE DENTRO L'IF, UN FIGLIO SA IL PROPRIO INDICE */
        if((pidFiglio = fork()) < 0){
            perror("Errore fork");
            exit(errno);
        }

        if(pidFiglio == 0)
        {   /* codice figlio */
            printf("Sono il figlio n° %d, il mio pid è %d\n", i, getpid());
            exit(i); /* il figlio ritorna il proprio indice */   
        }
    }

    /* wait dei figli */

    int ritorno;
    for(i = 0; i < N; i++)
    {   
        if((pidFiglio = wait(&status)) < 0){
            perror("Errore fork");
            exit(errno);
        }
        else 
        {   /* figlio terminato, sia normale che anomalo*/

            ritorno = (int)((status >> 8) & 0xFF);
            printf("PADRE: figlio con pid %d terminato con codice %d\n", pidFiglio, ritorno);
        }
    }

	exit(0);
}

/*
    NON È DETERMINISTICO L'ORDINE CON CUI I FIGLI TERMINANO 
    PER QUESTO L'OUTPUT NON È MOLTO CHIARO
    
*/