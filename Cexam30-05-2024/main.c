/*SOLUZIONE GIANLUCA RAVAGLIA - 2^ PROVA IN ITINERE 30/05/2024
    UTILIZZATO c17 nel makefile
*/

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

#define PERM 0644 /*Definisco i permessi da usare con creazione figlio*/
typedef int pipe_t[2]; /*Definisco tipo di dato per pipe*/

int main(int argc, char** argv){
    /*====================================VARIABILI================================================*/
    
    int pid; /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipes; /*array di pipe per comunicazione da figlio a padre per risultato grep*/
    pipe_t pipeFN; /*pipe locale di comunicazione tra figlio e nipote*/
    int FC; /*File descriptor per interagire con file di output*/

    int j = 0; /*Variabile contatore per scorrere la linea STORE*/
    char STORE[250]; /*Variabile per contenere le linee ricevute dai figli*/
    
    /*Stringa che indica come invocare il programma in caso di errore da parte dell'utente*/
    char* USAGE = "USAGE: ./main InteroPositivoY nomeFileSemplice\n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri -> devono essere esattamente 2 (!= 3 in quanto argc conta anche nome file in posizione 0)*/
    if (argc != 3)
    {
        printf("Errore: hai inserito %d parametri, necessari esattamente 2 parametri\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }

    /*Converto primo parametro tramite atoi per poi controllare che il valore sia un intero positivo*/
    int Y = atoi(argv[1]);
    if(Y <= 0){
        printf("Errore: il primo parametro inserito (%s) non è un numero intero strettamente positivo!\n", argv[1]);
        printf("%s\n", USAGE);
        exit(2);
    }

    /*Nome di file non va controllato*/

    /*Allocazione memoria per array di pipe tra padre e figli: ci sono Y figli quindi Y pipe da allocare*/
    if ((pipes = malloc(Y * sizeof(pipe_t))) == NULL) /*Controllo che l'allocazione sia andata a buon fine*/
    {
        printf("Errore nella allocazione di memoria per le pipe\n");
        exit(3);
    }

    /*Creo pipe per comunicazione tra padre e figli - l'altra pipe sarà creata nel figlio localmente*/
    for (int y=0; y < Y; y++)
    {
       if (pipe(pipes[y]) < 0) /*Controllo se la pipe ha restituito codice di errore*/
       {
          printf("Errore pipe\n");
          exit(4);
       }
    }
    
    /*TERMINE CONTROLLI ED OPERAZIONI PRELIMINARI*/
    //printf("DEBUG: Controlli ed operazioni preliminari concluse \n");
    
    /*Per prima cosa il padre crea/sovrascrive(se esistente) il file su cui verranno eseguiti gli output*/
    if ((FC = open(argv[2], O_CREAT|O_WRONLY|O_TRUNC, PERM)) < 0) /*Controllo che la creazione/sovrascrittura vada a buon fine*/
    {
        printf("Impossibile creare il file '%s'\n", argv[1]);
        exit(5);
    }

    /*Ora il padre crea Y processi figli*/
    /*=====================CODICE CREAZIONE FIGLI======================*/
    for(int y = 0; y < Y; y++){
        if((pid = fork()) < 0){ /*Effettuo fork e controllo se non va a buon fine*/
            printf("Errore nella fork\n");
            exit(6);
        }
    
        if(pid == 0){
            /*====CODICE FIGLI=====*/
            /*Per prima cosa i figli chiudono le pipe inutilizzate*/
            for(int j = 0; j < Y; j++){
                close(pipes[j][0]); /*Ogni figlio chiude il lato lettura della propria pipe*/
                if(j != y){
                    close(pipes[j][1]);   /*Solo nelle pipe altrui si chiude anche il lato lettura*/
                }
            }

            /*Creazione pipe di comunicazione con nipote*/
            if (pipe(pipeFN) < 0) /*Controllo se la pipe ha restituito codice di errore*/
            {
                printf("Errore pipe nel figlio di indice %d\n", y);
                exit(-1);
            }

            /*Creazione processo nipote*/
            if((pid = fork()) < 0){ /*controllo Errore fork*/
                printf("Errore nella fork nel figlio di indice %d\n", y);
                exit(-1);
            }
            if(pid == 0){
                /*===CODICE NIPOTE====*/
                close(pipeFN[0]); /*Chiudo lato lettura pipe con figlio*/
                close(pipes[y][1]); /*Chiudo pipe con padre che non viene usata dal nipote che non è già stata chiusa dal figlio*/

                close(2); /*Chiudo standard error per evitare stampe indesiderate a schermo*/
                
                /*Rigirigo standard output su pipe per inviare al figlio il risultato del comando ps*/
                close(1);
                /*Copio in 1 il file descriptor della pipe di scrittura al figlio*/
                dup(pipeFN[1]); /*Dup non controllata in quanto controllo superfluo come indicato via mail dalla prof*/

                /*Eseguo comando ps*/
                execlp("ps", "ps", (char*)0);

                /*A questo punto si arriva solo in caso di errore della execlp*/
                printf("Errore execlp nel nipote di indice %d\n", y);
                exit(-1);
                /*===FINE CODICE NIPOTE====*/
            }
            /*===RIPRENDE CODICE FIGLIO===*/
            close(pipeFN[1]); /*Chiudo lato lettura pipe con nipote in quanto devo soltanto leggere*/

            /*Ridirigo standard input, impostando la pipe di lettura dal nipote*/
            close(0);
            dup(pipeFN[0]); /*Controllo dup indicato come superfluo dalla prof*/
            /*Ridirigo standard output alla pipe rivolta al padre*/
            
            /*Ridirigo standard output sulla pipe di scrittura verso il padre*/
            close(1);
            dup(pipes[y][1]); /*controllo dup indicato superfluo dalla prof*/

            close(2); /*Chiudo standard error per evitare stampe di errori indesiderate*/

            /*Per passare il pid intero alla grep devo convertirlo in stringa*/
            char *strpid = malloc(4);
            sprintf(strpid, "%d", pid);

            /*A questo punto il figlio può eseguire il comando grep cercando la conversione a stringa del pid, individuando la linea del proprio nipote*/
            execlp("grep", "grep", strpid, (char*)0);

            /*Qui si arriva solo in caso di errore della execlp di grep*/
            printf("Errore execlp di grep nel figlio di indice %d\n", y);
            exit(-1);
        }
    }/*====================FINE CREAZIONE FIGLI========================*/
    
    /*====CODICE PADRE====*/
    /*Chiusura di tutte le pipe inutilizzate, ovvero le pipe in scrittura verso i figli*/
    for(int y = 0; y < Y; y++){
        close(pipes[y][1]);
    }

    /*Ora il padre riceve, seguendo l'ordine dei figli, le linee selezionate*/
    for(int y = 0; y < Y; y++){
        j = 0; /*Resetto a 0 per ogni linea che scorro in lettura*/
        /*Leggo carattere per carattere le linee inviate dai figli*/
        while(read(pipes[y][0], &(STORE[j]), 1) == 1){
            j++;
            if(STORE[j-1] == '\n'){/*Quando raggiungo il file linea interrompo lettura*/
                break;
            }
        }
        /*Ora possiedo linea ricevuta e sua lunghezza (compreso terminatore di linea)*/

        /*Scrivo su file di output la linea ricevuta, conoscendone la lunghezza posso scriverla in una write sola*/
        if(write(FC, STORE, j) != j){ /*Controllo che vengano scritti j byte*/
            printf("Errore del padre nella scrittura su file di output\n");
            exit(7);
        }
    }

    /*A questo punto il padre può attendere il termine dei propri figli ed effettuare stampe conseguentemente a cosa viene ritornato*/
    /*====================CODICE ATTESA FIGLI========================*/
    for(int y = 0; y < Y; y++){
        if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
            printf("Errore wait\n");
            exit(8);
        }
        if((status & 0xFF) != 0){ /*Caso terminazione anomala del figlio*/
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else{ /*Caso terminazione normale del figlio*/
            ritorno = (int)((status >> 8) & 0xFF);
            /*Controllo che la grep abbia ritornato 0 per effettuare la stampa di successo o insuccesso*/
            if(ritorno != 0) /*Caso insuccesso*/
                printf("Nel figlio di pid = %d si è verificato un problema della grep; ha ritornato %d\n", pidFiglio, ritorno);
            else /*Caso successo*/
                printf("Il figlio di pid = %d è terminato correttamente e ha ritornato %d\n", pidFiglio, ritorno);
        }
    }/*====================FINE ATTESA FIGLI========================*/
    
    exit(0);
}


