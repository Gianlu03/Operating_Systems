#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h> /*Libreria per gestire segnali*/
#include <stdbool.h>

#define PERM 0644;     /*Definisco i permessi da usare con creazione figlio*/
typedef int pipe_t[2]; /*Definisco tipo di dato per pipe*/


int main(int argc, char **argv)
{
    int pid;                       /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipes;                  /*Array di pipe*/
    char token = 't';               /*token da inviare su pipe come ok*/
    char carLetto;
    int occorrenze = 0;
    int fd; /*Descrittore file da utilizzare in interazioni con file*/

    /*Stringa da stampare in caso di errore di passaggio dei parametri da utente*/
    char* USAGE = "USAGE: ./13lug22 F L car1 car2 [... carn]\n";


    /*Controllo numero parametri*/
    if (argc < 5)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 5\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }
    int Q = argc-3; /*Variabile per memorizzare numero figli*/

    /*Controllo intero positivo*/
    if(atoi(argv[2]) <= 0){
        printf("Errore: il terzo parametro deve essere un intero positivo, inserito %s\n", argv[2]);
        printf("%s\n", USAGE);
        exit(2);
    }
    int L = atoi(argv[2]);

    /*Controllo che i parametri a partire dal terzo siano tutti caratteri*/
    for(int q = 0; q < Q; q++){
        if(strlen(argv[q+2]) != 1){
            printf("Errore: il parametro %s non è un carattere\n", argv[q+2]);
            printf("%s\n", USAGE);
            exit(3);
        }
    }

    /*Allocazione memoria per array di pipe*/
    if(((pipes = malloc((Q+1) * sizeof(pipe_t))) == NULL)){
        printf("Errore nella allocazione di memoria per le pipe\n");
        exit(4);
    }

    /*Ciclo creazione pipes*/
    for(int q = 0; q < Q+1; q++){
        if(pipe(pipes[q]) < 0){
            printf("Errore nella creazione delle pipe\n");
            exit(5);
        }
    }

    /*Tutti i parametri corretti*/
    printf("DEBUG: parametri corretti\n\n");
    
    /*Ciclo di creazione dei figli*/
    for(int q = 0; q < Q; q++){
        if((pid = fork()) < 0){
            printf("Errore nella fork\n");
            exit(6);
        }

        if(pid == 0){
            /*Codice figli*/
            /*Per prima cosa chiudo le pipe inutilizzate*/
            for(int j = 0; j < Q+1; j++){
                if(j != q)
                    close(pipes[j][0]);
                if(j != (q+1))
                    close(pipes[j][1]);
            }
            
            /*Controllo se riesco ad aprire il file, altrimenti errore*/
            if((fd = open(argv[1], O_RDONLY)) < 0){
                printf("Errore apertura in lettura del file %s", argv[1]);
                exit(-1);
            }
            /*Ogni figlio conta il numero di occorrenze nella riga e prima di stampare attende segnale da pipe*/
            while(read(fd, &carLetto, 1)){
                if(carLetto == '\n'){ /*Se raggiungo la fine di una riga attendo token da processo precedente, stampo e invio token al successivo*/
                    if(read(pipes[q][0], &token, 1) != 1){
                        printf("Errore lettura da pipe %d\n", q);
                        exit(-1);
                    }
                    printf("Figlio %d: ho letto %d occorrenze di %c\n", getpid(), occorrenze, argv[q+3][0]);
                    if(write(pipes[q+1][1], &token, 1) != 1){
                        printf("Errore scrittura su pipe %d\n", q+1);
                        exit(-1);
                    }
                    ritorno = occorrenze; //memorizzo ultimo conteggio per poter ritornare al padre con exit
                    occorrenze = 0;
                    continue;
                }
                if(carLetto == argv[q+3][0])
                    occorrenze++;
            }
            exit(ritorno);
        }
    }
    /*Fine creazione figli*/

    /*CODICE PADRE*/

    /*Per prima cosa chiude le pipe*/
    for(int i = 0; i < Q+1; i++){
        if(i != 0)
            close(pipes[i][1]);
        if(i != Q)
            close(pipes[i][0]);
    }
        
    
    for(int i = 0; i < L; i++){
        printf("Padre: Lettura linea %d\n", i+1);
        if(write(pipes[0][1], &token, 1) != 1){
            printf("Errore write nel padre\n");
            exit(7);
        }
        printf("padre ha mandato token\n");
        sleep(1);
        if(read(pipes[Q][0], &token, 1) != 1){
            printf("Errore read token nel padre \n");
            exit(8);
        }
    }

    /*CODICE ATTESA FIGLI*/
    for(int q = 0; q < Q; q++){
        if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
            printf("Errore wait\n");
            exit(7);
        }

        if((status & 0xFF) != 0){ /*Caso terminazione anomala*/
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else{ /*Caso terminazione normale*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
        }
    }/*FINE ATTESA FIGLI*/

    printf("Fine programma\n\n");

    exit(0);
}
