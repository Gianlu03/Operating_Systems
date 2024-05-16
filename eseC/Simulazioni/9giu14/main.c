#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<ctype.h>
#include<time.h>
#include<fcntl.h>
#include<unistd.h>

#define PERM 0644;
typedef int pipe_t[2];


int main(int argc, char** argv){
    int pid; /* array ritorno fork, necessito di array per memorizzare pid per stampare figlio con conteggio maggiore */
    int pidFiglio, ritorno, status; /* Variabili wait */
    pipe_t* piped; /* Array (dinamico) delle pipe padre-figli*/
    pipe_t pipeNipote; /* pipe tra figlio e nipote */
    int N = argc - 1; /* Variabile per numero parametri */
    char* USAGE = "USAGE: ./main fileAssol1 fileAssol2 [fileAssolN]\n"; /* Variabile con stringa da stampare di USAGE in caso di errore nei parametri*/

    int val;
    long int somma = 0; /*Valore intero passato tra le pipe e somma valori ricevuti da padre*/
    char buffer[10]; /*buffer per leggere i caratteri da pipe figlio-nipote*/
    
    /* Controllo numero parametri lasco: almeno 3*/
    if(N < 2){
        printf("Errore: necessari almeno 2 parametri, ne sono stati inseriti %d \n", N);
        printf("%s", USAGE);
        exit(1);
    }

    printf("DEBUG: numero parametri corretto\n");

    printf("DEBUG: PARAMETRI CORRETTI\n");

    /*Alloco la memoria necessaria per l'array di pipe, allo stesso tempo controllo che avvenga con successo*/
    if((piped = malloc(sizeof(pipe_t)*N)) == NULL){ 
        printf("Si è verificato un errore nella malloc delle pipe\n");
        exit(3);
    }

    /*Creo le pipe tra padre e figli prima di effettuare fork*/
    for(int i = 0; i < N; i++){ /*Sono N perchè c'è anche tra ultimo figlio e padre*/
        if(pipe(piped[i]) < 0){
            printf("Si è verificato un errore nella creazione delle pipe\n");
            exit(4);
        }
    }

    /* EFFETTUO LE FORK e definisco codice di padre e figli*/
    for(int i = 0; i < N; i++){
        /*Controllo che la fork vada a buon fine*/
        if((pid = fork()) < 0){
            printf("Si è verificato un errore nella fork()\n");
            exit(5);
        }

        /*Codice figli*/
        if(pid == 0){
            
            /*Per prima cosa attuo chiusura pipe non usate*/
            for(int j = 0; j < N; j++){
                /*Chiudo ogni lato lettura e lato scritture sulle pipe diverse dalla propria*/
                close(piped[j][0]);
                if(j != i) //pipe diversa dalla propria
                    close(piped[j][1]);
            }

            /*Adesso creo una pipe figlio-nipote*/
            if(pipe(pipeNipote) < 0){
                printf("Si è verificato un errore nella creazione della pipe con nipote\n");
                exit(-1);
            }
             /*Chiudo subito lato pipe inutilizzato*/

            /*Ora creo nipote tramite fork()*/
            if((pid = fork()) < 0){
                printf("Si è verificato un errore nella fork di creazione nipote\n");
                exit(-1);
            }

            if(pid == 0){/*CODICE NIPOTE*/
                /*Chiudo subito lato pipe inutilizzato*/
                close(pipeNipote[0]);
                //ridirigo standard input
                close(0);
                if(open(argv[i+1], O_RDONLY) < 0){
                    printf("Errore nell'apertura del file %s in lettura\n", argv[i+1]);
                    exit(-1);
                }

                //Ridirigo standard output
                close(1);
                dup(pipeNipote[1]);
            
                /*Eseguo comando filtro wc per contare linee file e scriverle in pipe*/
                execlp("wc", "wc", "-l", (char*)0);

                /*Qui arrivo in caso di errore*/
                printf("Errore execlp di wc\n");
                exit(-1);
            }
            /*Chiudo lato pipe inutilizzato, non prima del nipote o non può settarlo come stdout*/
            close(pipeNipote[1]);

            /*CODICE FIGLIO*/
            int j = 0;
            /*LEGGO DA PIPE UN CARATTERE ALLA VOLTA*/
            while(read(pipeNipote[0], &(buffer[j]), 1)){
                if(buffer[j] == '\0'){ /* incontro terminatore */
                    j++;
                    buffer[j] = '\n'; /*Cambio terminatore e interrompo ciclo*/
                    break;
                }
                j++;
            }

            /*Converto valore ricevuto*/
            if((val = atoi(buffer)) < 0){
                printf("Errore conversione valore wc\n");
                exit(-1);
            }

            /*Invio valore al padre*/
            write(piped[i][1], &val, sizeof(int));
                
            exit(val); //ritorno il valore anche tramite exit alla wait
        }

    }/* FINE CREAZIONE FIGLI */

    for(int i = 0; i < N; i++){ /*Padre legge e somma i valori ritornati dai figli*/
        read(piped[i][0], &val, sizeof(int));
        somma += val;
    }

    printf("La somma delle linee contate dai nipoti vale: %ld\n", somma);

    /*CODICE ATTESA FIGLI*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){
            printf("Errore wait\n");
            exit(7);
        }

        if((status & 0xFF) != 0){
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else{
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
        }
    }

    printf("Termine programma\n\n");
    exit(0);
}
