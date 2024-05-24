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
    
    int *pid; /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipePadreFiglio; /*array di pipe per comunicazione da padre a figlio*/
    pipe_t *pipeFiglioPadre; /*array di pipe per comunicazione da figlio a padre*/
    int fd; /*variabile per contenere file descriptor e interagire con file*/
    char token = 'y'; /*Carattere da fornire come informazione minima nella pipe per continuare/fermare algoritmo*/
    char carLetto; /*Variabile di buffer che contiene carattere letto*/

    char* USAGE = "USAGE: ./main file1 file2 [...fileN] fileF \n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri*/
    if (argc < 4)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 3\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }
    int N = argc-2; /*Variabile per mantenere numero figli*/

    /*Non effettuo controlli su file in quanto la correttezza è garantita dalla parte shell*/

    /*Allocazione memoria per array di pipe da figli a padre*/
    if ((pipeFiglioPadre = malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe dai figli al padre\n");
        exit(2);
    }

    /*Allocazione memoria per array di pipe da padre a figli*/
    if ((pipePadreFiglio = malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe dal padre ai figl\n");
        exit(3);
    }

    /*Allocazione memoria per array di pid*/
    if ((pid = malloc(N * sizeof(int))) == NULL)
    {
        printf("Errore nella allocazione di memoria per l'array dei pid\n");
        exit(4);
    }

    /*Creo le pipe controllando il successo dell'operazione*/
    for(int i = 0; i < N; i++){
        if(pipe(pipePadreFiglio[i]) < 0){
            printf("Errore nella creazione delle pipe da padre a figlio\n");
            exit(5);
        }
    
        if(pipe(pipeFiglioPadre[i]) < 0){
            printf("Errore nella creazione delle pipe da figlio a padre\n");
            exit(6);
        }
    }


    /*=====================CODICE CREAZIONE FIGLI======================*/
    for(int i = 0; i < N; i++){
        if((pid[i] = fork()) < 0){
            printf("Errore nella fork\n");
            exit(7);
        }
    
        if(pid[i] == 0){
            /*Codice figli*/
            /*Chiusura di tutte le pipe inutilizzate*/
            for (int j = 0; j < N; j++)
            {
                /*Ogni figlio chiude lettura della pipe in cui scrive e scrittura della pipe in cui legge*/
                close(pipeFiglioPadre[j][0]);
                close(pipePadreFiglio[j][1]);
                if (j != i)
                { /*Nelle pipe diverse dalle proprie i figli chiudono tutto*/
                    close(pipeFiglioPadre[j][1]);
                    close(pipePadreFiglio[j][0]);
                }
            }

            /*Ogni figlio apre il proprio file*/
            /*Tento apertura file controllando se va a buon fine o meno*/
            if((fd = open(argv[i+1], O_RDONLY)) < 0){
                printf("Errore apertura in lettura del file %s\n", argv[i+1]);
                exit(-1);
            }

            /*Effettuo operazioni finchè il padre manda il token*/
            while(read(pipePadreFiglio[i][0], &token, 1)){
                /*1 - Il figlio controlla che token ha ricevuto*/
                if(token == 'n') break; 
                /*2- Il figlio legge il proprio carattere dal file*/
                read(fd, &carLetto, 1); /*Si ipotizza tutti stessa lunghezza quindi non controllo*/
                /*3- Il figlio invia al padre il carattere letto per effettuare confronto*/
                if(i == 0)printf("%c", carLetto);
                write(pipeFiglioPadre[i][1], &carLetto, 1);
            }

            exit(0); /*Qui arrivano solo i figli con file uguali, ritornano successo*/
            /*I figli che non sono arrivati a questa exit saranno terminati da SIGKILL*/
        }
    }
    /*====================FINE CREAZIONE FIGLI========================*/
    
    /*CODICE PADRE*/
    /*Tento apertura file controllando se va a buon fine o meno*/

    /*Chiusura di tutte le pipe inutilizzate*/
    for(int i = 0; i < N; i++){
        close(pipeFiglioPadre[i][1]); /*Chiude scrittura della pipe da cui legge*/
        close(pipePadreFiglio[i][0]); /*Chiude lettura dalla pipe su cui scrive*/
    }

    /*Apertura di file e controllo - se il file non è apribile termino il programma killando tutti i figli*/
    if((fd = open(argv[argc-1], O_RDONLY)) < 0){
        printf("Errore apertura in lettura del file %s, termino i figli e il processo\n", argv[argc-1]);
        for(int i = 0; i < N; i++)
            kill(pid[i], SIGKILL); /*Se non riesce l'apertura del file del padre killo tutti i figli*/
        exit(8);
    }

    int* stop = calloc(sizeof(int), N); /*Array per memorizzare chi può continuare e chi no 1 = fermo, 0 = continua*/
    char ricevuto;
    /*Finchè il padre riesce a leggere caratteri continua il codice*/
    while(read(fd, &carLetto, 1)){
        for (int i = 0; i < N; i++)
        {
            if(stop[i] == 1) continue; /*Se il figlio deve stare fermo, passo al successivo*/
            /*1 - Il padre manda il segnale per leggere il carattere successivo*/
            if(write(pipePadreFiglio[i][1], &token, 1) != 1){
                printf("Problema scrittura in pipe del padre\n");
                exit(8);
            }

            /*2- Il padre attende il carattere letto dal figlio*/
            if(read(pipeFiglioPadre[i][0], &ricevuto, 1) != 1){
                printf("Problema lettura in pipe del padre\n");
                exit(9);
            }

            /*3- Il padre controlla se il carattere ricevuto dal figlio è diverso da quello letto*/
            if(carLetto != ricevuto){ /*Se diverso, il figlio non deve continuare, cambio l'array "continua"*/
                stop[i] = 1;
            }
        }
    }

    /*Quando il padre esce manda il token di terminazione a chi non ha trotato caratteri diffeerenti
      mentre termina con SIGKILL i figli con file differenti*/
    token = 'n';
    for(int i = 0; i < N; i++){
        if(stop[i] == 1) /*Se il figlio si è dovuto fermare, lo "killo"*/
            kill(pid[i], SIGKILL);
        else /*Se il figlio è arrivato in fondo lo termino correttamente*/
            write(pipePadreFiglio[i][1], &token, 1);
    }

    
        
    char* fileCorrispondente;
    /*====================CODICE ATTESA FIGLI========================*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
            printf("Errore wait\n");
            exit(10);
        }

        /*Devo trovare il suo file corrispondente per stamparlo*/
        for(int j = 0; j < N; j++){
            if(pid[j] == pidFiglio)
                fileCorrispondente = argv[j+1];
        }

        if((status & 0xFF) != 0){ /*Caso terminazione anomala*/
            printf("Figlio di pid = %d terminato in modo anomalo: il suo file %s non coincide a %s\n", pidFiglio, fileCorrispondente, argv[argc-1]);
        }
        else{ /*Caso terminazione normale*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d (se 255 problemi): il suo file %s coincide a %s\n", pidFiglio, ritorno, fileCorrispondente, argv[argc-1]);
        }
    }/*====================FINE ATTESA FIGLI========================*/
    

    exit(0);
}


