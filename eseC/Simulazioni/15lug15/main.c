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

void handler(int signo){
    printf("PADRE ha ricevuto SIGPIPE, termine programma\n");
    exit(8);
}


int main(int argc, char** argv){
    /*====================================VARIABILI================================================*/
    
    int* pid; /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipePadreFiglio; /*array di pipe per comunicazione da padre a figlio*/
    pipe_t *pipeFiglioPadre; /*array di pipe per comunicazione figlio a padre*/

    int fd; /*variabile per contenere file descriptor e interagire con file*/
    char token = 'v'; /*Carattere da fornire come informazione minima nella pipe per continuare/fermare algoritmo*/
    char carLetto; /*Variabile di buffer che contiene carattere letto*/
    int fileOutput; /*variabile per file descriptor di 'merge'*/
    int nr = 1; /*Contiene ritorno di read/write*/

    char* USAGE = "USAGE: ./main file1 file2 [...filen]\n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri*/
    if (argc < 3)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 2 parametri che indicano nomi di file\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }
    int N = argc-1; /*variabile contenente numero di file (ovvero di processi figli)*/

    /*Allocazione memoria per array di pipe*/
    pipeFiglioPadre = malloc(N * sizeof(pipe_t));
    pipePadreFiglio = malloc(N * sizeof(pipe_t));

    /*Controllo che la allocazione di memoria sia avvenuta con successo*/
    if(pipeFiglioPadre == NULL || pipePadreFiglio == NULL){
        printf("Errore nella allocazione di memoria per le pipe\n");
        exit(2);
    }

    /*Creo le pipe controllando il successo dell'operazione in entrambe*/
    for(int i = 0; i < N; i++){
        if(pipe(pipePadreFiglio[i]) < 0){
            printf("Errore nella creazione delle pipe da padre a figlio\n");
            exit(3);
        }
    
        if(pipe(pipeFiglioPadre[i]) < 0){
            printf("Errore nella creazione delle pipe da figlio a padre\n");
            exit(4);
        }
    }

    /*Allocazione memoria per array di pid*/
    if ((pid = malloc(N * sizeof(int))) == NULL)
    {
        printf("Errore nella allocazione di memoria per l'array dei pid\n");
        exit(5);
    }

    printf("DEBUG: Controlli ed operazioni preliminari effettuate con successo\n");
    
    
    /*Creo il file 'merge' su cui vengono scritti i caratteri*/
    if ( (fileOutput = open ("merge", O_CREAT | O_WRONLY | O_TRUNC, PERM)) < 0 )    {
        printf("Impossibile creare il file merge\n");
        exit(6);
    }
    printf("DEBUG: file creato con successo \n");
    
    /*Installo handler di Sigpipe ricevuto se un file termina per file non apribile in lettura*/
    signal(SIGPIPE,handler);

    /*=====================CODICE CREAZIONE FIGLI======================*/
    for(int i = 0; i < N; i++){
        if((pid[i] = fork()) < 0){
            printf("Errore nella fork\n");
            exit(7);
        }
    
        if(pid[i] == 0){
            /*Codice figli*/
            /*Chiusura di tutte le pipe inutilizzate*/
             for (int j = 0; j < N; j++){
                /*Ogni figlio chiude lettura della pipe in cui scrive e scrittura della pipe in cui legge*/
                close(pipeFiglioPadre[j][0]);
                close(pipePadreFiglio[j][1]);
                if (j != i)
                { /*Nelle pipe diverse dalle proprie i figli chiudono tutto*/
                    close(pipeFiglioPadre[j][1]);
                    close(pipePadreFiglio[j][0]);
                }
            }

            /*Tento apertura file controllando se va a buon fine o meno*/
            if((fd = open(argv[i+1], O_RDONLY)) < 0){
                printf("Errore apertura in lettura del file %s da parte del figlio %d\n", argv[i+1], i);
                exit(-1);
            }

            /*1- Attendo che il padre mandi token di ok per leggere carattere*/
            while(read(pipePadreFiglio[i][0], &token, 1)){
                printf("DEBUG:letto token\n");
                
                /*2-Leggo carattere da file: se file finito termino figlio*/
                if(read(fd, &carLetto, 1) != 1){
                    exit(carLetto); /*Figlio che termina invia ultimo carattere*/
                }
                /*3-Invio carattere letto al padre*/
                if(write(pipeFiglioPadre[i][1], &carLetto, 1) != 1){
                    printf("Errore invio carattere al padre nel figlio %d", i);
                    exit(-1);
                }
            }

            exit(carLetto);
        }
    }/*====================FINE CREAZIONE FIGLI========================*/
    
    /*CODICE PADRE*/
    /*Chiusura di tutte le pipe inutilizzate*/
    sleep(2);
    for(int i = 0; i < N; i++){
        close(pipeFiglioPadre[i][1]); /*Chiude scrittura della pipe da cui legge*/
        close(pipePadreFiglio[i][0]); /*Chiude lettura dalla pipe su cui scrive*/
    }
    
    /*Il ciclo di letture termina una volta che un figlio ritorna un numero di byte = 0*/
    while(nr > 0){ 
        for (int i = 0; i < N; i++)
        {
            /*1-Il padre invia il token di ok per leggere*/
            if(write(pipePadreFiglio[i][1], &token, 1) != 1){
                printf("Problema nel padre all'invio del token\n");
                //exit(8); richiamata SIGPIPE
            }
            /*2-Il padre attende di ricevere il carattere dalla pipe*/
            nr = read(pipeFiglioPadre[i][0], &carLetto, 1);
            /*Se viene ricevuto un carattere*/
            if(nr > 0){
                /*3-Scrivo sul file merge il carattere ricevuto (controllo vada a buon fine)*/
                if(write(fileOutput, &carLetto, 1) != 1){
                    printf("Problema scrittura su 'merge' da parte del padre\n");
                    exit(9);
                }
            }
            else{ /*Ho ricevuto 0 byte dalla read: significa che un figlio è terminato*/
                /*4-Con terminazione di un figlio, uccido tutti i figli tramite SIGKILL*/
                for(int j = 0; j < N; j++)
                    kill(pid[j], SIGKILL); 
                break; /*Termino for per andare subito nel while che si bloccherà per nr = 0*/
            }
        }
    }

    /*====================CODICE ATTESA FIGLI========================*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
            printf("Errore wait\n");
            exit(10);
        }
        if((status & 0xFF) != 0){ /*Caso terminazione anomala*/
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else{ /*Caso terminazione normale*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d(%c) (se 255 problemi)\n", pidFiglio, ritorno, (char)ritorno);
        }
    }/*====================FINE ATTESA FIGLI========================*/
    
    exit(0);
}


