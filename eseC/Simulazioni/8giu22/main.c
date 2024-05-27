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

#define MSGSIZE 4 /*Dimensione massima righe dei file (compreso terminatore linea)*/

typedef int pipe_t[2]; /*Definisco tipo di dato per pipe*/

void handler(int singo){
    printf("Ricevuto segnale SIGPIPE: un figlio è terminato prima di scrittura da parte di P0, P0 termina\n");
    exit(-1);
}

int main(int argc, char** argv){
    /*====================================VARIABILI================================================*/
    
    int pid; /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipes; /*array di pipe per comunicazione da figlio p0 a figlio pi*/
    int fd; /*variabile per contenere file descriptor e interagire con file*/

    char linea[MSGSIZE]; /* Stringa contenente linea letta dal proprio file*/
    char buffer[MSGSIZE]; /* Stringa contenente linea ricevuta da figlio p0*/

    char* USAGE = "USAGE: ./main file1 file2 file3 [...fileN]\n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri*/
    if (argc < 4)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 3\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }
    int N = argc - 1; /*Numero di file (Ovvero numero di processi figli)*/

    /*Il padre non necessita di pipe per comunicare*/
    /*Necessarie per comunicare da figlio p0 a figli pi, quindi N-1 canali*/

    /*Allocazione memoria per array di pipe*/
    if ((pipes = malloc((N-1) * sizeof(pipe_t))) == NULL)
    {
        printf("Errore nella allocazione di memoria per le pipe\n");
        exit(2);
    }

    /*Creazione pipe con controllo che vada a buon fine*/
    for (int n = 0; n < N; n++)
    {
       if (pipe(pipes[n]) < 0) /*Exit se non è creata correttamente*/
       {
          printf("Errore pipe\n");
          exit(3);
       }
    }

    /*Controlli ed operazioni preliminari completate*/
    printf("DEBUG: Controlli ed operazioni preliminari concluse con successo\n");
    
    /*=====================CODICE CREAZIONE FIGLI======================*/
    for(int n = 0; n < N; n++){
        if((pid = fork()) < 0){
            printf("Errore nella fork\n");
            exit(4);
        }
    
        if(pid == 0){/*Codice figli*/

            /*Tutti i figli tentano apertura del proprio file*/
            if((fd = open(argv[n+1], O_RDONLY)) < 0){
                printf("Errore apertura in lettura del file %s da parte del figlio di indice %d\n", argv[n+1], n);
                exit(-1);
            }            

            /*Codice primo figlio p0*/
            if(n == 0){

                /*Primo figlio chiude tutte le pipe in lettura, in scrittura le usa tutte*/
                for(int j = 0; j < N - 1; j++)
                    close(pipes[j][0]);
                /*1- P0 legge la propria linea*/
                while(read(fd, linea, MSGSIZE) == MSGSIZE){ /*Tutte le linee sono supposte di lunghezza msgsize*/
                    /*2-P0 invia la propria linea a tutti i fratelli*/
                    linea[MSGSIZE-1] = '\0'; /*Converto terminatore di linea in term. di stringa*/
                    for(int j = 0; j < N-1; j++){
                        if(write(pipes[j][1], linea, MSGSIZE) != MSGSIZE){ /*Controllo vada a buon fine la scrittura*/
                            printf("Errore nell'invio della linea da parte del figlio di indice %d\n", n);
                            /*QIU VIENE LANCIATO IL SIGPIPE che stampa e termina figlio p0*/
                        }
                    }

                }
                
            }
            else{ /*Codice figli successivi*/
                /*I figli successivi chiudono le seguenti pipe:*/
                for(int j = 0; j < N-1; j++){
                    close(pipes[j][1]); /*Chiude tutte le pipe in scrittura, sia la propria che quelle altrui*/
                    if(j != n-1) /*La propria pipe ha indice n-1*/
                        close(pipes[j][0]); /*Chiude quelle altrui in lettura per chiuderle completamente*/
                }

                /*1- Ricevono linea da P0 memorizzandola in buffer finchè ne vengono mandate*/
                while(read(pipes[n-1][0], buffer, MSGSIZE) == MSGSIZE){
                
                    /*qui linea ricevuta correttamente da p0*/
                    /*2- Per ogni linea letta dal proprio file...*/
                    while(read(fd, linea, MSGSIZE) == MSGSIZE){
                        linea[MSGSIZE-1] = '\0';
                        /*3-Figlio crea nipote*/

                        if((pid = fork()) < 0){
                            printf("Errore fork in figlio di indice %d\n", n);
                            exit(-1);
                        }
                        if(pid == 0){ /*CODICE NIPOTE*/
                            close(pipes[n][0]);
                            close(1); /*Chiudo standard output ed error per evitare stampe, mi interessa il valore di ritorno in $?*/
                            close(2);

                            /*4- Il nipote esegue comando diff, il valore sarà ritornato come exit*/
                            execlp("diff", "diff", buffer, linea, (char*)0); /*Ritorna 0 se file uguali*/

                            /*Qui si arriva solo in caso di problemi con execlp*/
                            printf("Si è verificato un problema in execlp in un nipote di indice %d\n", n);
                            exit(-1);
                        }

                        /*il figlio conclude attendendo nipote*/
                        if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
                            printf("Errore wait in figlio di indice %d\n", n);
                            exit(-1);
                        }
                        if((status & 0xFF) != 0){ /*Caso terminazione anomala*/
                            printf("Nipote di pid = %d e indice %d terminato in modo anomalo\n", pidFiglio, n);
                        }
                        else{ /*Caso terminazione normale*/
                            ritorno = (int)((status >> 8) & 0xFF);
                            if(ritorno == 0)
                                printf("I file %s e %s sono uguali\n", buffer, linea);
                        }
                    }
                    lseek(fd, 0, SEEK_SET); /*Per ogni file ricevuto da P0 deve leggere il file da capo*/
                }
                printf("finito di ricevere\n");
            }
            exit(n); /*Ogni figlio ritorna il proprio indice d'ordine*/
        }
    }/*====================FINE CREAZIONE FIGLI========================*/
    
    /*CODICE PADRE - deve solo attendere i figli e stampare pid e valore di ritorno*/
    for(int i = 0; i < N-1; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /*====================CODICE ATTESA FIGLI========================*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){ /*Errore wait*/
            printf("Errore wait\n");
            exit(5);
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


