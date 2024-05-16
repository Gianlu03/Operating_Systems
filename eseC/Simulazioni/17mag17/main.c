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

typedef struct {
    long int c1;
    int c2;
    long int c3;
}s_occ;

int main(int argc, char** argv){
    int *pid; /* array ritorno fork, necessito di array per memorizzare pid per stampare figlio con conteggio maggiore */
    int pidFiglio, ritorno, status; /* Variabili wait */
    pipe_t* piped; /* Array (dinamico) delle pipe */
    int N = argc - 2; /* Variabile per numero parametri */
    char Cx; /* Variabile per contenere carattere (ultimo parametro)*/
    int fd; /* Variabile per contenere file descriptor*/
    char* USAGE = "USAGE: ./main filename1 filename2 [filenamen] Carattere\n"; /* Variabile con stringa da stampare di USAGE in caso di errore nei parametri*/
    s_occ mess; /*Definisco struct per inviare messaggio sulla pipe*/


    /* Controllo numero parametri lasco: almeno 3*/
    if(argc < 2+2){
        printf("Errore: necessari almeno 3 parametri, ne sono stati inseriti %d\n", argc - 1);
        printf("%s", USAGE);
        exit(1);
    }

    printf("DEBUG: numero parametri corretto\n");

    /*Controllo che l'ultimo parametro sia un carattere singolo*/
    if(strlen(argv[argc-1]) != 1){
        printf("Errore: l'ultimo parametro (%s) non è un carattere \n", argv[argc-1]);
        printf("%s", USAGE);
        exit(2);
    }
    printf("DEBUG: PARAMETRI CORRETTI\n");
    Cx = argv[argc-1][0];

    /*Effettuo e controllo allocazione array di int*/
    if((pid = malloc(sizeof(int)*N)) == NULL){ 
        printf("Si è verificato un errore nella malloc dei pid\n");
        exit(3);
    }

    /*Alloco la memoria necessaria per l'array di pipe, allo stesso tempo controllo che avvenga con successo*/
    if((piped = malloc(sizeof(pipe_t)*N)) == NULL){ 
        printf("Si è verificato un errore nella malloc delle pipe\n");
        exit(4);
    }

    /*Creo le pipe prima di effettuare fork*/
    for(int i = 0; i < N; i++){ /*Sono N perchè c'è anche tra ultimo figlio e padre*/
        if(pipe(piped[i]) < 0){
            printf("Si è verificato un errore nella creazione delle pipe\n");
            exit(5);
        }
    }

    /* EFFETTUO LE FORK e definisco codice di padre e figli*/
    for(int i = 0; i < N; i++){
        /*Controllo che la fork vada a buon fine*/
        if((pid[i] = fork()) < 0){
            printf("Si è verificato un errore nella fork()\n");
            exit(6);
        }

        /*Codice figli*/
        if(pid[i] == 0){
            /*ogni figlio tenta apertura file, in caso di errore termina*/
            if((fd = open(argv[i+1], O_RDONLY)) < 0){
                printf("Impossibile aprire file %s, verificarne l'esistenta e/o i diritti di accesso\n", argv[i+1]);
                exit(-1);
            }

            char ch; /* variabile buffer per carattere letto*/
            long int occorrenze = 0; /* Contatore occorrenze carattere richiesto*/

            /*Devo differenziare il codice sulla base dell'indice, dato che il primo figlio non legge la struct e chiude diversamente le pipe*/
            if(i == 0){ /*codice primo figlio*/
                /* Il primo figlio chiude tutto tranne scrittura sulla piped 0*/
                close(piped[0][0]);
                for(int j = 1; j < N; j++){
                    close(piped[j][0]);
                    close(piped[j][1]);
                }
                mess.c3 = 0; /*Inizializzo valore somma*/
                mess.c2 = i; /*Metto in automatico indice primo figlio, in caso non fosse il massimo verrà sostituito*/
                /*Non leggo struct da pipe, lo effettuano solo i figli successivi*/
                
            }else{
                /*Chiusura pipe come prima cosa*/
                close(piped[i-1][1]); /*Chiude scrittura pipe precedente a sè*/
                close(piped[i][0]); /*Chiude lettura pipe successiva a sè*/
        
                for(int j = 0; j < N; j++){
                    if(j != i && j != (i-1)){ /*le pipe non adiacenti sono chiuse completamente*/
                        close(piped[j][0]);
                        close(piped[j][1]);
                    }
                }

                /*Controllo se il numero di byte letti corrisponde alla dimensione della struct*/
                if(read(piped[i-1][0], &mess, sizeof(s_occ)) != sizeof(s_occ)){
                    printf("Figlio %d: Lettura errata in piped[%d]\n", getpid(), i-1);
                    exit(-1);
                }
            }
            /*Codice comune a tutti i figli*/

            /*Scorro il file corrispondente e conto occorrenze leggendo 1 carattere per volta*/
                while(read(fd, &ch, 1)){
                    if(ch == Cx)
                        occorrenze++;
                }
                mess.c3 += occorrenze; /* Aggiungo conteggio alla somma*/
                if(occorrenze > mess.c1){
                    /*Se il conteggio supera il massimo fino ad ora diventa il nuovo massimo*/
                    mess.c2 = i; 
                    mess.c1 = occorrenze; 
                }
                //printf("DEBUG: figlio %d ha contato %ld\n", getpid(), occorrenze);

                /*Scrivo su pipe struttura dati*/
                write(piped[i][1], &mess, sizeof(s_occ));
                //printf("DEBUG: figlio %d ha scritto su pipe\n", i);
                exit(i);
        }

    }/* FINE CREAZIONE FIGLI */

    /*codice padre*/
    sleep(3);
    close(piped[N-1][1]); /*Chiudo scrittura ultima pipe(da cui legge)*/
    for(int i = 0; i < N-1; i++){
        /*Chiudo completamente altre pipe con cui non interagisce*/
        close(piped[i][0]);
        close(piped[i][1]);
    }

    /*Controllo che legga un numero di byte corretto*/
    if(read(piped[N-1][0], &mess, sizeof(mess)) != sizeof(mess)){
        printf("padre %d: Lettura errata in piped[%d]\n", getpid(), N-1);
        exit(7);
    }

    printf("\n|Padre: Stampo i dati ricevuti riguardo al carattere '%c'|\n", Cx);
    printf("=======================================================================================\n");
    printf("-Il massimo conteggio del carattere è: %ld\n", mess.c1);
    printf("-Il massimo conteggio avvenuto nel file -> %s\n", argv[mess.c2+1]);
    printf("-Il conteggio massimo è stato effettuato dal figlio di pid = %d e indice ordine = %d\n", pid[mess.c2], mess.c2);
    printf("-La somma di tutti i conteggi è: %ld\n", mess.c3);
    printf("=======================================================================================\n");


    /*CODICE ATTESA FIGLI*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){
            printf("Errore wait\n");
            exit(8);
        }

        if((status & 0xFF) != 0){
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else{
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d\n", pidFiglio, ritorno);
        }
    }

    printf("Termine programma\n\n");
    exit(0);
}
