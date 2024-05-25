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

#define PERM 0644; /*Definisco i permessi da usare con creazione figlio*/
typedef int pipe_t[2]; /*Definisco tipo di dato per pipe*/

int main(int argc, char** argv){
    /*====================================VARIABILI================================================*/
    
    int* pid; /*Variabile per ritorno fork (necessito dei pid nel padre quindi uso un array)*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipes; /*array di pipe per comunicazione tra processi*/
    int fd; /*variabile per contenere file descriptor e interagire con file*/
    int L = 0; /*variabile per contare lunghezza linea*/
    char carLetto; /*Variabile di buffer che contiene carattere letto da file*/
    int X; /*Variabile per contenere intero richiesto ad utente*/
    int lineaAttuale = 1; /*variabile utilizzata dai figli per contare a quale riga si trovano in cerca della riga X*/

    /*N.B L'INDICE DELLA PRIMA RIGA È INTESO COME 1*/

    char linea[255];
    char* USAGE = "USAGE: ./main file1 file2 [...filen] interoPositivoX\n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri*/
    if (argc < 4)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 3\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }

    /*Controllo intero positivo*/
    if(atoi(argv[argc-1]) <= 0){
        printf("Errore: il terzo parametro deve essere un intero positivo, inserito %s\n", argv[argc-1]);
        printf("%s\n", USAGE);
        exit(2);
    }
    int K = atoi(argv[argc-1]); /*Intero positivo chiamato K come richiesto*/
    int N = argc-2; /*Variabile per contenere numero di file (e di figli)*/

    /*Allocazione memoria per array di pipe con controllo successo*/
    if ((pipes = malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe\n");
        exit(3);
    }

    /*Creo le pipe controllando che l'operazione vada a buon fine*/
    for(int i = 0; i < N; i++){
        if(pipe(pipes[i]) < 0){
            printf("Errore nella creazione delle pipe\n");
            exit(4);
        }
    }

    /*Allocazione memoria per array di pid con controllo successo operazione*/
    if ((pid = malloc(N * sizeof(int))) == NULL)
    {
        printf("Errore nella allocazione di memoria per l'array dei pid\n");
        exit(5);
    }

    printf("DEBUG: parametri e operazioni preliminari concluse\n");

    /*Come da testo, prima il padre richiede X in ingresso, poi crea figli che ereditano X nell'area dati*/
    printf("Utente, inserisci un numero intero positivo minore o uguale a %d (1-%d)", K, K);
    scanf("%d", &X);

    /*Controllo che il valore X inserito sia positivo e <= K*/
    if(X <= 0 || X > K){
        printf("Inserito un valore X non compreso nel range 1-%d, termino programma\n", K);
        exit(6);
    }

    /*A questo punto posso creare i figli*/

    /*=====================CODICE CREAZIONE FIGLI======================*/
    for(int i = 0; i < N; i++){
        if((pid[i] = fork()) < 0){
            printf("Errore nella fork\n");
            exit(7);
        }
    
        if(pid[i] == 0){
            /*Codice figli*/
            /*Per prima cosa i figli chiudono le pipe inutilizzate*/
            for(int j = 0; j < N; j++){
                close(pipes[j][0]); /*Ogni figlio chiude il lato lettura della propria pipe*/
                if(j!=i){
                    close(pipes[j][1]);   /*Solo nelle pipe altrui chiude anche il lato lettura*/
                }
            }

            /*Il figlio apre il file (con controllo sul successo dell'operazione)*/
            if((fd = open(argv[i+1], O_RDONLY)) < 0){
                printf("Errore apertura in lettura del file %s nel figlio di indice %d\n", argv[i+1], i);
                exit(-1);
            }

            /*1- Il figlio cerca la riga X se non è la prima*/
            if(lineaAttuale != X){
                while(read(fd, &carLetto, 1)){ 
                    if(carLetto == '\n'){ /*Conto i \n incontrati per contare in quale riga mi sposto*/
                        lineaAttuale++;
                        if(lineaAttuale == X) /*Se ho raggiunto la linea esco dal ciclo*/
                            break;
                    }
                }
                /*Quando esco da questo ciclo potrei non aver trovato la linea X*/
                if(lineaAttuale != X){
                    L = -1;
                    if(write(pipes[i][1], &L, sizeof(int)) != sizeof(int)){
                        printf("Errore nel figlio %d quando scrive lunghezza L su pipe\n", i);
                        exit(-1);
                    }
                    
                    /*3-Il figlo scrive la linea (con controllo che vengano scritti L byte)*/
                    if(write(pipes[i][1], linea, 1) != 1){
                        printf("Errore nel figlio %d quando scrive lunghezza L su pipe\n", i);
                        exit(-1);
                    }
                        exit(0); /*Ritorna 0 come richiesto*/
                }
            }
            /*ELSE: sono già nella linea attuale*/

            /*2-Il figlio legge la riga carattere per carattere per individuarne la lunghezza*/
            while(read(fd, &carLetto, 1)){
                L++;
                if(carLetto == '\n'){ /*Quando raggiungo la fine della linea:*/
                    lseek(fd, -L, SEEK_CUR); /*Torno a inizio linea*/
                    if(read(fd, linea, L*sizeof(char)) != L*sizeof(char)){ /*La leggo in una volta sola nell'array*/
                        printf("Errore nella lettura della linea X in figlio %d\n", i);
                        exit(-1);
                    }
                    /*Il cursore si è già riposizionato a fine riga correttamente*/
                    /*Il figlio ora ha tutti i dati che deve inviare al padre (L poi linea)*/
                    /*2-figlio Scrive L (con controllo vengano scritti 4 byte di intero)*/
                    linea[L-1] = '\0'; /*Cambio il terminatore prima di scrivere su pipe*/
                    if(write(pipes[i][1], &L, sizeof(int)) != sizeof(int)){
                        printf("Errore nel figlio %d quando scrive lunghezza L su pipe\n", i);
                        exit(-1);
                    }
                    /*3-Il figlo scrive la linea (con controllo che vengano scritti L byte)*/
                    
                    if(write(pipes[i][1], linea, L) != L){
                        printf("Errore nel figlio %d quando scrive lunghezza L su pipe\n", i);
                        exit(-1);
                    }
                    /*ora il figlio può uscire dal ciclo di lettura*/
                    break;
                }
            }
            
            /*Ora il figlio ritorna la L individuata tramite exit*/
            exit(L);            

        }
    }/*====================FINE CREAZIONE FIGLI========================*/
    
    /*CODICE PADRE*/
    /*Per prima cosa il padre chiude le pipe inutilizzate*/
    for(int i = 0; i < N; i++)
        close(pipes[i][1]); /*Deve solo leggere quindi chiude i lati scrittura*/
    
    /*Il padre riceve le informazioni dei figli seguendo l'ordine dei file*/
    for (int i = 0; i < N; i++)
    {
        /*1-il padre riceve la lunghezza di linea L (con controllo ricezione lettura corretta di pipe)*/
        if(read(pipes[i][0], &L, sizeof(int)) != sizeof(int)){
            printf("Errore lettura da pipe[%d] da parte del padre in ricezione di L\n", i);
            exit(8);
        }

        /*2-Verifico se il figlio ha inviato lunghezza -1 indicando di non avere la linea richiesta*/
        if(L == -1){
            printf("Il figlio di pid = %d e indice %i non ha trovato la linea richiesta nel file '%s'\n", pid[i], i, argv[i+1]);
            continue; /*passa al prossimo figlip*/
        }

        /*3-Se la lunghezza indica che la linea è stata trovata, leggo la linea (con controllo)*/
        if(read(pipes[i][0], linea, L) != L){
            printf("Errore lettura da pipe[%d] da parte del padre in ricezione della linea\n", i);
            exit(9);
        }

        /*4-Ora che il padre ha tutto il necessario, stampa le informazioni*/
        printf("Il figlio di pid = %d e indice %i ha trovato nel file '%s' la seguente linea:\n", pid[i], i, argv[i+1]);
        printf("%s\n", linea);

        /*5-Passo al figlio successivo*/
    }
    
    /*Sleep per avere maggiore ordine tra le stampe*/
    sleep(2);
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
            printf("Il figlio di pid = %d ha ritornato %d (se 255 problemi - se 0 non ha trovato la linea)\n", pidFiglio, ritorno);
        }
    }/*====================FINE ATTESA FIGLI========================*/
    

    exit(0);
}


