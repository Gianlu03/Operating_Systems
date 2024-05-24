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
int *finito;
int N;

int finitof()
{
    /* questa funzione verifica i valori memorizzati nell'array finito: 
    appena trova un elemento uguale a 0 vuole dire che non tutti i processi figli sono 
    finiti e quindi torna 0; 
    tornera' 1 se e solo se tutti gli elementi dell'array sono a 1 
    e quindi tutti i processi figli sono finiti */
    int i;
    for (i=0; i < N; i++)
        if (!finito[i])
                /* appena ne trova uno che non ha finito */
            return 0; /* ritorna falso */
    return 1;/*Se hanno finito tutti ritorno vero*/
}

int main(int argc, char** argv){
    /*====================================VARIABILI================================================*/
    
    int pid; /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipePadreFiglio; /*array di pipe per comunicazione da padre a figlio*/
    pipe_t *pipeFiglioPadre; /*array di pipe per comunicazione da padre a figlio*/
    int fd; /*variabile per contenere file descriptor e interagire con file*/
    char ricevuto = 't'; /*Carattere da fornire come informazione minima nella pipe per continuare/fermare algoritmo*/
    char carLetto; /*Variabile di buffer che contiene carattere letto*/
    long int posizione = 0;
    int contaModifiche = 0;

    char* USAGE = "./main  USAGE: file1 [...filen] CarattereCx\n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri*/
    if (argc < 3)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 2\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }
    N = argc-2;

    /*Controllo singolo carattere su ultimo parametro */
    if(strlen(argv[argc-1]) != 1){
        printf("Errore: il parametro %s non è un carattere\n", argv[argc-1]);
        printf("%s\n", USAGE);
        exit(2);
    }
    char Cx = argv[argc-1][0];/*Memorizzo ultimo carattere in una variabile come richiessto*/

    /*Allocazione memoria per array di pipe*/
    if ((pipeFiglioPadre = malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe da figlio a padre");
        exit(3);
    }

    /*Allocazione memoria per array di pipe*/
    if ((pipePadreFiglio = malloc(N * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe da padre a figlio\n");
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
        if((pid = fork()) < 0){
            printf("Errore nella fork\n");
            exit(7);
        }
    
        if(pid == 0){
            /*Codice figli*/
            /*Chiusura di tutte le pipe inutilizzate per evitare deadlock*/
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
            if((fd = open(argv[i+1], O_RDWR)) < 0){
                printf("Errore apertura in lettura del file %s di indice = %d\n", argv[i+1], i);
                exit(-1);
            }

            /*1- Effettuo lettura carattere per carattere*/
            while(read(fd, &carLetto, 1)){
                /*2- Se trovo corrispondenza invio posizione al padre*/
                if(carLetto == Cx){
                    /*3- Il figlio invia al padre */
                    if(write(pipeFiglioPadre[i][1], &posizione, sizeof(long int)) != sizeof(long int)){
                        printf("Si è verificato un problema nel figlio %d ad inviare la posizione\n", i);
                        exit(-1);
                    }
                    /*4- il figlio riceve dal padre il carattere con cui sostituire*/
                    if(read(pipePadreFiglio[i][0], &ricevuto, 1) != 1){
                        printf("Si è verificato un problema nel figlio %d nel ricevere il carattere con cui sostituire\n", i);
                        exit(-1);
                    }
                    /*5- il figlio attua sostituzione(se non \n)*/
                    if(ricevuto != '\n'){
                        lseek(fd, -1, SEEK_CUR); //indietreggio di un posto
                        write(fd, &ricevuto, 1); //Qui il cursore torna in posizione con scrittrua
                        contaModifiche++;
                    }
                }
                posizione++; //Alla fine aumento la posizione per prossimo carattere
            }
            /*Una volta terminato, il figlio invia -1 come posizione per indicare che è terminato*/
            posizione = -1L;

            write(pipeFiglioPadre[i][1], &posizione, sizeof(long int)); /*invio -1 in segno di termine*/
            /*6- Il figlio termina ritornando le occorrenze modificate*/
            exit(contaModifiche);
        }
    }/*====================FINE CREAZIONE FIGLI========================*/
    
    /*CODICE PADRE*/
    /*Chiusura di tutte le pipe inutilizzate dal padre*/
    for(int i = 0; i < N; i++){
        close(pipeFiglioPadre[i][1]); /*Chiude lato scrittura della pipe da cui legge*/
        close(pipePadreFiglio[i][0]); /*Chiude lato lettura dalla pipe su cui scrive*/
    }

    /*Alloco array di indicatori di termine dei figli e controllo che sia avvenuta con successo*/
    finito = calloc(sizeof(int), N); /*Array per indicare se i figli sono terminati o no: 0=in corso  1=terminato*/
    if(finito == NULL){
        printf("Errore nella allocazione di memoria dell'array di indicatori di termine\n");
        exit(8);
    }

    char scarto;
    /*Il ciclo termina una volta che tutti i figli sono conclusi*/
    while(!finitof()){
        for(int i = 0; i < N; i++){
            /*Il codice è svolto solo se il figlio non è concluso*/
            if(finito[i] == 0){ /*Se il figlio non è terminato faccio le richieste*/
                /*1- Il padre attende che il figlio fornisca la posizione*/
                if(read(pipeFiglioPadre[i][0], &posizione, sizeof(long int)) != sizeof(long int)){
                    printf("Si è verificato un problema nel padre leggendo la posizione dalla pipe[%d]\n", i);
                    exit(9);
                }
                /*Se il figlio ha inviato il valore di termine lo memorizzo come concluso e passo al figlio successivo*/
                if(posizione < 0){
                    finito[i] = 1;
                    continue;
                }
                else{ /*Se non è concluso attuo le richieste del testo*/
                /*1- Chiede ad utente quale carattere inserire*/
                    //char input[255];
                    printf("Il figlio %i ha trovato '%c' in posizione %ld nel file '%s'\n", i, Cx, posizione, argv[i+1]);
                    printf("Con quale carattere lo vuoi sostituire? \n");
                    read(0, &carLetto, 1);
                    if(carLetto != '\n')
                        read(0, &scarto, 1); /*Effettuo una seconda lettura per eliminare \n*/
                /*2- fornisco al figlio il carattere ricevuto da utente*/
                    if(write(pipePadreFiglio[i][1], &carLetto, 1) != 1){
                        printf("Errore scrittura carattere da sostituire da parte del padre\n");
                        exit(11);
                    }
                }
            }
            /*ELSE: passo a figlio successivo*/
        }
    }
    /*Esco una volta che tutti i figli sono conclusi*/


    /*====================CODICE ATTESA FIGLI========================*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
            printf("Errore wait\n");
            exit(12);
        }
        if((status & 0xFF) != 0){ /*Caso terminazione anomala*/
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else{ /*Caso terminazione normale*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha effettuato %d sostituzioni(se 255 problemi)\n", pidFiglio, ritorno);
        }
    }/*====================FINE ATTESA FIGLI========================*/

    exit(0);
}


