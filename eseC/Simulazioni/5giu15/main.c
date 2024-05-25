#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<ctype.h>
#include<time.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h> /*Libreria per gestire segnali*/
#include<stdbool.h>

typedef int pipe_t[2]; /*Definisco tipo di dato per pipe*/

int main(int argc, char** argv){
    /*====================================VARIABILI================================================*/
    
    int pid; /*Variabile per ritorno delle fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipes1; /*array di pipe per comunicazione tra padre e figli*/
    pipe_t *pipes2; /*array di pipe per comunicazione tra figli e nipoti*/
    int lunghezza = 0; /*Variabile per contenere il numero di caratteri contato da ogni figlio*/
    char carLetto; /*Variabile buffer per contenere carattere letto da read*/
    int fd; /*variabile per contenere file descriptor e interagire con file*/


    char* USAGE = "USAGE: ./main \n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri: almeno 2*/
    if (argc < 3)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 2\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }
    int M = argc-1; /*Variabile che contiene il numero di parametri*/

    /*Allocazione memoria per array di pipe*/
    if ((pipes1 = malloc(M * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe padre-figlio\n");
        exit(2);
    }
    /*Allocazione memoria per array di pipe*/
    if ((pipes2 = malloc(M * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe figli-nipoti\n");
        exit(3);
    }

    for (int j = 0; j < M; j++)
    {
        /*Creo e verifico la corretta creazione delle pipe*/
        if(pipe(pipes1[j]) < 0){
            printf("Errore nella creazione delle pipe padre-figlio\n");
            exit(4);
        }
        /*Creo e verifico la corretta creazione delle pipe*/
        if(pipe(pipes2[j]) < 0){
            printf("Errore nella creazione delle pipe figlio-nipote \n");
            exit(5);
        }
    }


    printf("Preparazioni preliminari concluse\n");

    /*=====================CODICE CREAZIONE FIGLI======================*/
    for(int j = 0; j < M; j++){
        if((pid = fork()) < 0){
            printf("Errore nella fork per creare figli\n");
            exit(6);
        }
    
        if(pid == 0){
            /*Codice figli*/
            /*Per prima cosa i figli chiudono le pipe inutilizzate*/
            for(int i = 0; i < M; i++){
                close(pipes1[i][0]); /*Ogni figlio chiude il lato lettura della propria pipe*/
                if(j!=i){
                    close(pipes1[i][1]);   /*Solo nelle pipe altrui si chiude anche il lato lettura*/
                    close(pipes2[i][0]);
                    close(pipes2[i][1]); /*Delle pipe estranee chiudo tutto*/
                }
            }/*La pipe con il proprio nipote verrà chiusa dopo il codice del nipote per permettergli di usarla*/

            /*Tento apertura file controllando se va a buon fine o meno*/
            if((fd = open(argv[j+1], O_RDONLY)) < 0){
                printf("Errore apertura in lettura del file '%s' da parte del figlio di indice %d\n", argv[j+1], j);
                exit(-1);
            }

            /*A questo punto ogni figlio crea il proprio nipote*/
            if((pid = fork()) < 0){
                printf("Errore nella fork per creare nipote\n");
                exit(-1);
            }

            if(pid == 0){
                /*CODICE NIPOTE*/

                /*Chiusura pipe (ereditata chiusura di pipe estranee)*/
                close(pipes2[j][0]); /*Chiude lato lettura della propria pipe su cui vuole solo scrivere*/

                /*Il nipote deve eseguire il comando tail sul file:*/
                /*Ridireziono standard input sul file da cui leggere*/
                close(0);
                if(open(argv[j+1], O_RDONLY) < 0){ /*Verifico se possibile aprire in lettura tale file*/
                    printf("Errore in apertura file '%s' in lettura da parte del nipote di indice %d", argv[j+1], j);
                    exit(-1);
                }
                /*Ridireziono standard output sulla pipe di comunicazione con il figlio che ha creato il nipote corrente*/
                close(1);
                if(dup(pipes2[j][1]) < 0){
                    printf("Errore dup: impossibile ridirezionare output nipote %d su pipe", j);
                    exit(-1);
                }

                /*Una volta ridirezionato, il nipote può eseguire il comando tail -1*/
                execlp("tail", "tail", "-1", (char *)0);

                /*A questo codice si arriva solo in caso di problemi con la exec*/
                printf("Si è verificato un problema nell'exec del nipote di indice %d", j);
                exit(-1);
            }
            /*CODICE FIGLIO*/
            /*chiudo pipe rimanente*/
            close(pipes2[j][1]);

            /*Il figlio ora recupera tramite questa pipe la linea individuata dal nipote*/
            /*Effettuo lettura carattere per carattere per contare la lunghezza della linea*/
            while(read(pipes2[j][0], &carLetto, 1)){
                if(carLetto == '\n'){
                    break; /*Non conto lo \n*/
                }
                lunghezza++;
            }

            /*A questo punto invio al padre tramite pipe la lunghezza calcolata*/
            if(write(pipes1[j][1], &lunghezza, sizeof(int)) != sizeof(int)){
                printf("Non è stato possibile scrivere correttamente la lunghezza su pipe dal figlio %d", j);
                exit(-1);
            }
            /*rimane solo da riportare al padre l'exit del nipote*/


            /*Alla fine il processo figlio attende nipote per riportare al padre il codice con cui è terminato*/
            if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
                printf("Errore wait\n");
                exit(-1);
            }
            if((status & 0xFF) != 0){ /*Caso terminazione anomala*/
                printf("Nipote di pid = %d terminato in modo anomalo\n", pidFiglio);
            }
            else{ /*Caso terminazione normale*/
                ritorno = (int)((status >> 8) & 0xFF);
                printf("Il nipote di pid = %d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
            }
        
            exit(ritorno);
        }
    }/*====================FINE CREAZIONE FIGLI========================*/
    
    /*CODICE PADRE*/
    /*Per prima cosa il padre chiude le pipe inutilizzate*/
    for(int j = 0; j < M; j++){
        close(pipes2[j][0]); /*Chiude totalmente le pipe di interazione con nipoti*/
        close(pipes2[j][1]);
        close(pipes1[j][1]);/*Chiude solo lato lettura delle pipe condivise con i figli*/
    }

    /*Il padre riceve e stampa in ordine a quello dei file le informazioni ricevute*/
    for(int j = M-1; j >= 0; j--){
        if(read(pipes1[j][0], &lunghezza, sizeof(int)) != sizeof(int)){
            printf("Errore nella lettura da pipe da parte del padre\n");
            exit(7);
        }

        printf("Il figlio di indice %d ha contato una lunghezza di %d caratteri nel file'%s'\n", j, lunghezza, argv[j+1]);
    }

    /*Sleep per maggiore ordine nell'output*/
    sleep(1);
    /*====================CODICE ATTESA FIGLI========================*/
    for(int i = 0; i < M; i++){
        if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
            printf("Errore wait\n");
            exit(8);
        }
        if((status & 0xFF) != 0){ /*Caso terminazione anomala*/
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else{ /*Caso terminazione normale*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
        }
    }/*====================FINE ATTESA FIGLI========================*/
    

    exit(0);
}