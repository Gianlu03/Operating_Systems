#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<ctype.h>
#include<time.h>
#include<fcntl.h>
#include<unistd.h>

typedef int pipe_t[2];
#define PERM 0644


int main(int argc, char* argv[]){
    int* pid1; /* Array per memorizzare PID figli, PID nipoti e stampare in ordine */
    int* pid2;
    int pidFiglio, ritorno, status; /*Variabili per wait*/

    pipe_t *pipeFigli; /*Pipe per comunicazione tra padre e figli*/
    pipe_t *pipeNipoti; /*Pipe per comunicazione tra padre e nipoti*/

    int fd; /*Variabile per contenere file descriptor dei file passati come parametro*/
    char carLetto; /*Variabile buffer per contenere il carattere letto*/
    long int contaTrasformazioni = 0; /*Conta quante occorrenze da modificare vengono trovate da un figlio/nipote*/

    char* USAGE = "USAGE: ./main file1 file2 [filen]\n"; /*Stampa di USAGE in caso di errore da utente nei parametri*/

    /*Controllo numero parametri lasco: almeno 2*/
    if(argc < 3){
        printf("Errore: inseriti %d parametri, necessari almeno 2\n", argc-1);
        printf("%s", USAGE);
        exit(1);
    }

    int N = argc-1; /*Variabile per numero parametri senza eseguibile*/

    /*Proceso allocando gli array*/
    if((pid1 = malloc(sizeof(int)*N)) == NULL){ /*Creo array lungo il doppio con pid di figli e nipoti*/
        printf("Errore nella malloc dei pid dei figli\n");
        exit(2);
    }
    if((pid2 = malloc(sizeof(int)*N)) == NULL){ /*Creo array lungo il doppio con pid di figli e nipoti*/
        printf("Errore nella malloc dei pid dei nipoti\n");
        exit(3);
    }

    if((pipeFigli = malloc(sizeof(pipe_t) * N)) == NULL){
        printf("Errorre nella malloc delle pipe dei figli\n");
        exit(4);
    }
    if((pipeNipoti = malloc(sizeof(pipe_t) * N)) == NULL){
        printf("Errorre nella malloc delle pipe dei nipoti\n");
        exit(5);
    }

    /*Il padre crea in primis tutte le pipe*/
    for(int i = 0; i < N; i++){ /*Creazione pipe con figli*/
        if(pipe(pipeFigli[i]) < 0){
            printf("Errore nella creazione delle pipe dei figli\n");
            exit(6);
        }
    }
    for(int i = 0; i < N; i++){
        if(pipe(pipeNipoti[i]) < 0){ /*Creazione pipe nipoti*/
            printf("Errore nella creazione delle pipe dei nipoti\n");
            exit(7);
        }
    }

    /*Ora il padre può procedere creando i figli*/

    /*CODICE CREAZIONE FIGLI*/
    for(int i = 0; i < N; i++){
        /*Per prima cosa ogni figlio chiude tutte le pipe inutilizzate(mantiene anche la pipe del nipote corrispondente
        altrimenti non potrà aprirla!!)*/

        /*effettuo fork*/
        if((pid1[i] = fork()) < 0){
            printf("Errore nella fork\n");
            exit(6);
        }

        if(pid1[i] == 0){ /*CODICE FIGLIO*/
            /*Chiudo pipe inutilizzate*/
            for(int j = 0; j < N; j++){
                if(j != i){ /*Chiudo tutte le pipe degli altri figli/nipoti */
                    close(pipeFigli[j][0]);
                    close(pipeFigli[j][1]);
                    close(pipeNipoti[j][0]);
                    close(pipeNipoti[j][1]);
                }
            }
            /*In posizione N il pid del primo nipote*/
            if((pid2[i] = fork()) < 0){
                printf("Errore fork per nipoti\n");
                exit(-1);
            }

            if(pid2[i] == 0){ /*CODICE NIPOTI*/
                /*Chiudo pipe inutilizzate non chiuse prima*/
                close(pipeFigli[i][0]);
                close(pipeFigli[i][1]);
                close(pipeNipoti[i][0]); /*Chiudo propria pipe in lettura*/

                /*Nipote e figlio aprono file in 2 momenti separati per avere 2 fd indipendenti*/
                if((fd = open(argv[i+1], O_RDWR)) < 0){
                    printf("Errore apertura in lettura di '%s' da parte di nipote\n", argv[i+1]);
                    exit(-1);
                }

                while(read(fd, &carLetto, 1)){ /*Se viene individuato un alfabetico minuscolo*/
                    if(isalpha(carLetto) && islower(carLetto)){
                        contaTrasformazioni++; //aumento conteggio
                        lseek(fd,-1L, SEEK_CUR); //indietreggio di un carattere
                        carLetto = toupper(carLetto); //rendo il carattere maiuscolo
                        write(fd, &carLetto, 1); //sovrascrivo con carattere maiuscolo
                    }
                }

                /*Scrivo numero occorrenze trovate su pipe corrispondente*/
                write(pipeNipoti[i][1], &contaTrasformazioni, sizeof(long int));

                exit(contaTrasformazioni/256); /*Ritorno valori richiesti che corrispondono a risultati di questa divisione intera*/
            }

            /*RIPRENDE CODICE FIGLIO*/
            close(pipeNipoti[i][0]); /*Chiudo pipe nipote rimasta*/
            close(pipeNipoti[i][1]);

            if((fd = open(argv[i+1], O_RDWR)) < 0){
                printf("Errore apertura in lettura di '%s' da parte di nipote\n", argv[i+1]);
                exit(-1);
            }

            while(read(fd, &carLetto, 1)){ /*Se viene individuato un alfabetico minuscolo*/
                if(isdigit(carLetto)){
                    contaTrasformazioni++; //aumento conteggio
                    lseek(fd,-1L, SEEK_CUR); //indietreggio di un carattere
                    write(fd, " ", 1); //sovrascrivo con carattere spazio vuoto
                }
            }

            /*Scrivo numero occorrenze trovate su pipe corrispondente*/
            write(pipeFigli[i][1], &contaTrasformazioni, sizeof(long int));

            /*Anche i figli attendono i nipoti*/
            if((pidFiglio = wait(&status)) < 0){ /*Controllo errore della wait*/
                printf("Errore wait\n");
                exit(7);
            }

            if((status & 0xFF) != 0){ /*Controllo se il figlio è terminato in modo anomalo*/
                printf("Nipote di pid = %d terminato in modo anomalo\n", pidFiglio);
            }
            else{ /*Figlio terminato correttamente, stampo i suoi dati*/
                ritorno = (int)((status >> 8) & 0xFF);
                printf("Il nipote di pid = %d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
            }


            exit(contaTrasformazioni/256); /*Ritorno valori richiesti che corrispondono a risultati di questa divisione intera*/
        }
    }/*FINE CREAZIONE FIGLI*/

    /* CODICE PADRE*/
    /*chiudo tutte le pipe in scrittura*/
    sleep(3);
    for(int i = 0; i < N; i++){
        close(pipeFigli[i][1]);
        close(pipeNipoti[i][1]);
    }

    printf("\n");
    for(int i = 0; i < N; i++){
        /*Stampo file trasformato, quante trasformazioni sono svolte e da quali processi*/
        printf("Operazioni eseguite sul file %s di indice %d:\n", argv[i+1], i);
        read(pipeFigli[i][0], &contaTrasformazioni, sizeof(long int));
        printf("Figlio di pid = %d ha effettuato %ld trasformazioni di caratteri numerici\n", pid1[i], contaTrasformazioni);
        read(pipeNipoti[i][0], &contaTrasformazioni, sizeof(long int));
        printf("Nipote di pid = %d ha effettuato %ld trasformazioni di caratteri minuscoli\n", pid2[i], contaTrasformazioni);
        printf("\n");    
    }

    /*CODICE ATTESA FIGLI*/
    for(int i = 0; i < N; i++){
        if((pidFiglio = wait(&status)) < 0){ /*Controllo errore della wait*/
            printf("Errore wait\n");
            exit(7);
        }

        if((status & 0xFF) != 0){ /*Controllo se il figlio è terminato in modo anomalo*/
            printf("Figlio di pid = %d terminato in modo anomalo\n", pidFiglio);
        }
        else{ /*Figlio terminato correttamente, stampo i suoi dati*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d (se 255 problemi)\n", pidFiglio, ritorno);
        }
    }/*FINE ATTESA FIGLI*/

    exit(0);
}