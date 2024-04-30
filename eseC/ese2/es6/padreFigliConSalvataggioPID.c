/* FILE COPIATO DA: status1.c */
#include <stdio.h> //Standard I/O: printf, BUFSIZ
#include <fcntl.h> //File CoNTrol: open, O_RDONLY, O_WRONLY, O_RDWR
#include <stdlib.h> //STanDard LIBrary: exit, malloc, calloc, free, atoi
#include <unistd.h> //UNIx Standard: read, write, close, SEEK_SET, SEEK_CUR, SEEK_END
#include <string.h> //STRING: strlen, strcpy, strcat
#include <sys/stat.h> //SYStem STATus: stat, fstat, S_IFMT, S_IFDIR, S_IFREG
#include <sys/wait.h> //SYStem WAIT: wait
#include <errno.h> //errno
#include <time.h> //srand(time(NULL))


int main (int argc, char* argv[])
{
    srand(time(NULL)); /* seed per intero random */
    int N; //variabile per contenere parametro da utente
    int status, pidFiglio, ritorno; /* variabili per valori di ritorno wait */
    

    /* controllo numero parametri */
    if(argc != 2)
    {   /* num parametri errato, deve essere esattamente 1 */
        printf("Non è stato inserito esattamente 1 parametro!\n");
        printf("USAGE: ./main interoPositivo\n");
        exit(1);
    }
    /*numero parametri corretto, proseguo*/
    

    /* Controllo parametro intero < 155 */
    if(atoi(argv[1]) <= 0 && atoi(argv[1]) < 155)
    { /*parametro errato, non è intero positivo*/
        printf("Non è stato inserito un parametro intero positivo!\n");
        printf("USAGE: ./main interoPositivo\n");
        exit(2);
    }
    N = atoi(argv[1]);

    printf("DEBUG: parametro corretto\n\n");

    printf("PADRE: pid = %d, N = %d\n", getpid(), N); /* stampo pid padre e intero in ingresso */

    int *pidFigli = malloc(sizeof(int) * N); /* array per memorizzare pid figli*/

    /* Creazione N figli */
    int i;
    for(i = 0; i < N; i++)
    {   /* ESSENDO IL CODICE DENTRO L'IF, UN FIGLIO SA IL PROPRIO INDICE */
        if((pidFiglio = fork()) < 0){
            perror("Errore fork");
            free(pidFigli);
            exit(3);
        }

        if(pidFiglio == 0)
        {   /* codice figlio */
            /* Ogni figlio stampa indice ordine, pid e ritorna un randomico*/
            printf("Sono il figlio n° %d, il mio pid è %d\n", i, getpid());
            exit(rand() % (101+i)); /* il figlio ritorna il random */   
        }

        /* CODICE PADRE: MEMORIZZA IL PID DI OGNI FIGLIO */
        pidFigli[i] = pidFiglio;
    }


    int indice;
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){ /* errore wait termina */
            printf("Errore nella wait\n");
            free(pidFigli);
            exit(4);
        }
        
        for(int j = 0; j < N; j++){
            if(pidFigli[j] == pidFiglio){
                indice = j;
                break;
            }
        }

        /* wait con insuccesso, stampo pid, indice ordine e indico terminazione anomala*/
        if((status & 0xFF) != 0){
            printf("Il figlio di pid = %d di indice d'ordine %d è terminato in modo anomalo\n", pidFigli[indice], indice);
        }
        else{
            /* wait con successo, stampo pid, indice e valore di ritorno*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d di indice d'ordine %d ha ritornato %d\n", pidFigli[indice], indice, ritorno);
        }
    }

    free(pidFigli);
	exit(0);
}
