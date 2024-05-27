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

/*Definisco variabili globali per funzione "finito"*/
int *finito;
int Q;

/*Funzione per verificare se ogni figlio ha terminato il proprio codice*/
int finitof(){
    for (int q = 0; q < Q; q++)
    {
        if(finito[q] == 0) /*Se trovo un elemento dell'array a 0 (=non finito) non ho finito*/
            return 0;
    }
    return 1;  /*Se tutti gli elementi sono a 1 ho finito e ritorno 1*/
}

int main(int argc, char** argv){
    /*====================================VARIABILI================================================*/
    
    int pid; /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipes; /*array di pipe per comunicazione tra processi*/
    int fd; /*variabile per contenere file descriptor e interagire con file*/
    long int contaLinee = 0; /*variabile per contare occorrenze*/
    char linea[250]; /*Linea supposta al massimo di 250 caratteri*/
    int j; /*variabile per cicli lettura carattere per carattere*/

    char* USAGE = "USAGE: ./main file1 file2 [...fileq]\n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri*/
    if (argc < 3)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 2\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }
    Q = argc-1;
    
    /*Allocazione memoria per array di pipe*/
    if ((pipes = malloc(Q * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe\n");
        exit(2);
    }

    /*creazione pipe con controllo successo*/
    for (int q = 0;q < Q; q++)
    {
       if (pipe(pipes[q]) < 0)
       {
          printf("Errore pipe\n");
          exit(3);
       }
    }

    /*Alloco array di controllo dei figli per codice del padre azzerato tramite calloc*/
    finito = calloc(sizeof(int), Q);
    if(finito == NULL){
        printf("Errore nella allocazione di memoria per array di controllo figli terminati\n");
        exit(4);
    }


    printf("DEBUG: Controlli ed operazioni preliminari completate \n");
    
    /*Per prima cosa il padre crea il file di output Camilla (se esiste lo svuota)*/
    if ((fd = open("Camilla", O_CREAT|O_WRONLY|O_TRUNC, PERM)) < 0)
    {
        printf("Impossibile creare il file Camilla\n");
        exit(5);
    }
    printf("DEBUG: file creato \n");
    /*I figli sovrascriveranno fd nel loro codice con il file descriptor del file associato*/

    /*A questo punto il padre può creare i figli*/

    /*=====================CODICE CREAZIONE FIGLI======================*/
    for(int q = 0; q < Q; q++){
        if((pid = fork()) < 0){
            printf("Errore nella fork\n");
            exit(6);
        }
    
        if(pid == 0){
            /*Codice figli*/
            /*Come prima cosa si chiudono le pipe per evitare deadlock*/
            for(j = 0; j < Q; j++){
                close(pipes[j][0]); /*Ogni lato lettura viene chiuso (usato solo dal padre)*/
                if(j != q)
                    close(pipes[j][1]); /*Chiudo pipe in scrittura altrui*/
            }

            /*Tento apertura file controllando se va a buon fine o meno*/
            if((fd = open(argv[q+1], O_RDONLY)) < 0){
                printf("Errore apertura in lettura del file %s\n", argv[q+1]);
                exit(-1);
            }

            /*1-Il figlio legge la propria linea da file*/
            j=0;
            while(read(fd, &(linea[j]), 1)){
                if(linea[j] == '\n'){
                    j++;
                    /*2-Se il primo carattere è numerico invia la linea al padre*/
                    if(isdigit(linea[0])){
                        if(write(pipes[q][1], linea, j) != j){
                            printf("Errore scrittura linea da parte del figlio di indice %d", q);
                            exit(-1);
                        }
                        contaLinee++; /*conto le linee inviate per exit*/
                    }
                    j = 0;/*La lunghezza della linea riparte da 0 per la successiva*/
                    continue;
                }
                j++;
            }
            /*3- il figlio ritorna il numero di linee contate (<255)*/
            exit(contaLinee);
        }
    }/*====================FINE CREAZIONE FIGLI========================*/
    
    /*CODICE PADRE*/
    /*Per prima cosa chiude le pipe*/
    for(int q = 0; q < Q; q++){
        close(pipes[q][1]); /*Padre chiude tutte le pipe in lettura, deve solo leggere*/
    }

    printf("\n"); /*Spazio per distanziare da output precedenti*/
    while(!finitof()){/*Eseguo lettura finche non hanno finito tutti i figli*/
        for(int q = 0; q < Q; q++){
            if(finito[q] == 0){ /*Effettuo questo controllo perchè i figli possono finire in momenti diversi*/
                /*1-Ricevo linea dal figlio*/
                j=0;
                while (read(pipes[q][0], &(linea[j]), 1)){
                    if(linea[j] == '\n'){
                        linea[j] = '\0';
                        j++;
                        break; 
                    } 
                    j++;
                }

                if(j == 0) { /*Visto che una linea è ricevuta solo se inizia per un digit, se non vengono contati caratteri il figlio ha finito*/
                    finito[q] = 1;
                    continue;
                }

                /*Ho finito di ricevere la linea*/
                /*3-Scrive su Camilla -> so già che sono j byte*/
                if(write(fd, linea, j) != j){
                    printf("Problema nel padre a scrivere su Camilla\n");
                    exit(7);
                }

                /*4- Il padre effettua le stampe*/
                printf("Il figlio di indice %d ha trovato nel file '%s' la linea:", q, argv[q+1]);
                printf("%s\n", linea);
            }
            /*else passo al prossimo figlio*/
        }
    } 

    printf("\n");
    /*====================CODICE ATTESA FIGLI========================*/
    for(int q = 0; q < Q; q++){
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
    

    printf("\n");
    exit(0);
}


