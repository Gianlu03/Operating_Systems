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

#define PERM 0644
typedef int pipe_t[2]; /*Definisco tipo di dato per pipe*/

int mia_random(int n)
{
    int casuale;
    casuale = rand() % n;
    return casuale;
}

/*handler*/
void handler(int signo){
    printf("Un figlio non ha aperto correttamente il file, termine programma\n");
    //signal(signo, handler);
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    /*====================================VARIABILI================================================*/
    int pid;                        /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipePadreFiglio;        /*array di pipe per comunicazione da padre a figlio*/
    pipe_t *pipeFiglioPadre;        /*array di pipe per comunicazione da figlio a padre*/
    int fd;                         /*variabile per contenere file descriptor e interagire con file*/
    char carLetto;                  /*Variabile di buffer che contiene carattere letto*/

    int contaCaratteri = 0; /*Variabile per contare i caratteri dei file nei figli*/
    int *contatori;
    int rispostaPadre;
    int contaCaratteriScritti = 0;

    int fileTemp;

    char *USAGE = "USAGE: ./main file1 file2 file3 file4\n";
    /*==============================================================================================*/

    /*Controllo numero parametri*/
    if (argc < 6)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 5\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }

    /*Controllo intero positivo minore di 255*/
    if (atoi(argv[argc - 1]) <= 0 || atoi(argv[argc - 1]) > 255)
    {
        printf("Errore: il terzo parametro deve essere un intero positivo minore di 255, inserito %s\n", argv[argc - 1]);
        printf("%s\n", USAGE);
        exit(2);
    }
    int H = atoi(argv[argc - 1]); /*variabile che contiene lunghezza file*/
    int N = argc - 2;             /*Variabile per contenere numero file, ovvero il numero di figli necessari*/

    if ((fileTemp = open("creato", O_CREAT|O_WRONLY|O_TRUNC, PERM)) < 0)
    {
        printf("Impossibile creare il file /tmp/creato\n");
        exit(3);
    }

    /*Allocazione memoria per array di pipe da figlio a padre*/
    if ((pipeFiglioPadre = malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe\n");
        exit(4);
    }
    /*Allocazione memoria per array di pipe da padre a figlio*/
    if ((pipePadreFiglio = malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe\n");
        exit(5);
    }

    /*Creo le pipe controllando il successo dell'operazione*/
    for(int i = 0; i < N; i++){
        if(pipe(pipePadreFiglio[i]) < 0){
            printf("Errore nella creazione delle pipe da padre a figlio\n");
            exit(6);
        }

        if(pipe(pipeFiglioPadre[i]) < 0){
            printf("Errore nella creazione delle pipe da figlio a padre\n");
            exit(7);
        }
    }

    /*Allocazione memoria per array di contatori*/
    if((contatori = malloc((N) * sizeof(pipe_t))) == NULL){
        printf("Errore nella allocazione di memoria per l'array di contatori\n");
        exit(6);
    }

    /*Installo gestore SIGPIPE: se un figlio termina il padre riceve questo segnale se comunica su pipe senza consumatore*/
    signal(SIGPIPE, handler);

    /*=====================CODICE CREAZIONE FIGLI======================*/
    for (int i = 0; i < N; i++)
    {
        if ((pid = fork()) < 0)
        {
            printf("Errore nella fork\n");
            exit(5);
        }

        if (pid == 0)
        {
            /*Codice figli*/
            /*Chiusura di tutte le pipe inutilizzate*/
            for (int j = 0; j < N; j++)
            {
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
                printf("Errore apertura in lettura del file %s\n", argv[i+1]);
                exit(-1); /*Il padre riceverà SIGPIPE a causa della*/
            }

            /*Effettuo lettura carattere per carattere*/
            while(read(fd, &carLetto, 1)){
                /*1- Il figlio conta i caratteri della propria linea*/
                contaCaratteri++;
                
                /*2- Il figlio arriva a fine riga*/
                if(carLetto == '\n'){ 
                    
                    /*3- Il figlio invia la lunghezza della propria riga al padre*/
                    if(write(pipeFiglioPadre[i][1], &contaCaratteri, sizeof(int)) != sizeof(int)){
                        printf("Errore write: scrittura intero fallita\n");
                        exit(-1);
                    }

                    /*4- Attende e riceve l'indice del carattere da trascrivere sul file temporaneo dal padre (controlla lettura corretta)*/
                    if(read(pipePadreFiglio[i][0], &rispostaPadre, sizeof(int)) != sizeof(int)){
                        printf("Errore, non è stato possibile leggere correttamente l'intero da pipe\n");
                        exit(-1);
                    }

                    /*Solo se l'indice del padre lo permette, trascrivo quel carattere su "creato"*/                    
                    if(contaCaratteri >= rispostaPadre){
                        /*Riposiziono il puntatore all'inizio della riga*/                    
                        lseek(fd, -contaCaratteri, SEEK_CUR); //indietreggio del numero di caratteri contati
                        lseek(fd, rispostaPadre, SEEK_CUR); //mi posiziono nel carattere richiesto dal padre
                        read(fd, &carLetto, 1);
                        write(fileTemp, &carLetto, 1);
                        lseek(fd, -1+contaCaratteri-rispostaPadre, SEEK_CUR); //riporto lseek dove era arrivato prima di cercare il carattere indicato dal padreS
                        contaCaratteriScritti++; //aumento il numero di caratteri trovati
                    }
                        
                    contaCaratteri = 0; /*Contatore riparte sia in caso di lunghezza adatta che non*/
                } //fine if == '\n'
            }
            exit(contaCaratteriScritti);
        }
    } /*====================FINE CREAZIONE FIGLI========================*/

    /*CODICE PADRE*/
    /*Chiusura di tutte le pipe inutilizzate*/
    for(int i = 0; i < N; i++){
        close(pipeFiglioPadre[i][1]); /*Chiude scrittura della pipe da cui legge*/
        close(pipePadreFiglio[i][0]); /*Chiude lettura dalla pipe su cui scrive*/
    }
    
    /*Eseguo l'algoritmo per ogni riga del file*/
    for(int j = 0; j < H; j++){
        /*Padre riceve conteggi da tutti i figli*/
        for(int i = 0; i < N; i++){
            /*Padre riceve (controllando errori) tutti i conteggi svolti dai figli*/
            if(read(pipeFiglioPadre[i][0], &(contatori[i]), sizeof(int)) != sizeof(int)){
                printf("Errore nella lettura dei conteggi nel padre\n");
                sleep(4);
                exit(7);
            }
        }
        /*Una volta letti, il padre sceglie a random l'indice del figlio da cui prendere il valore massimo per il prossimo valore random*/
        /*Random tra 0 e un numero a caso tra i conteggi ritornati dai figli*/
        rispostaPadre = rand()%contatori[rand()%N];
        
        /*Padre invia valore randomico scelto a tutti i figli*/
        for (int i = 0; i < N; i++)
        {
            if(write(pipePadreFiglio[i][1], &rispostaPadre, sizeof(int)) != sizeof(int)){
                printf("Errore nell'invio della risposta da parte del padre\n");
                exit(8);
            }
        }
        printf("Padre: Il valore random è: %d\n", rispostaPadre);
    }


    /*====================CODICE ATTESA FIGLI========================*/
    for (int i = 0; i < N; i++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        { /*Errore wait*/
            printf("Errore wait\n");
            exit(7);
        }
        if ((status & 0xFF) != 0)
        { /*Caso terminazione anomala*/
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else
        { /*Caso terminazione normale*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
        }
    } /*====================FINE ATTESA FIGLI========================*/

    exit(0);
}
