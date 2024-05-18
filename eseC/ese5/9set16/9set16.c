/* File di risoluzione dell'esame C del 9 settembre 2016*/
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<ctype.h>
#include<time.h>
#include<fcntl.h>
#include<unistd.h>

#define PERM 0644; /* Definisco permessi per creazione file */
typedef int pipe_t[2]; /* Definisco tipo di dato per le pipe */
#define N 26

typedef struct{
    char v1; /*Carattere cercato*/
    long int v2;    /* occorrenze carattere */
} v_occ;

int main(int argc, char* argv[]){

    int pid[N]; /* Array per memorizzare pid dei figli */
    int pidFiglio, status, ritorno; /* Variabili per wait*/
    pipe_t pipes[N]; /*Array di pipe per comunicazione tra figli e con padre*/
    int fd; /*file descriptor del file parametro*/
    char carLetto; /*Variabile buffer per read nei figli*/    
    v_occ occorrenze[N];   
    char* USAGE = "USAGE: ./main fileAssoluto\n"; /*Stringa stampata in caso di errore da parte dell'utente nei parametri*/

    /*Controllo numero parametri: esattamente 2*/
    if(argc != 2){
        printf("Errore: sono stati forniti %d parametri. Necessario 1 singolo parametro\n", argc);
        printf("%s", USAGE);
        exit(1);
    }
    char* F = argv[1]; /* Memorizzo il file con nome richiesto */

    /*Per prima cosa il padre crea le pipe*/
    for(int i = 0; i < N; i++){
        if(pipe(pipes[i]) < 0){ /*In caso di errore nella pipe, exit*/
            printf("Errore nella creazione delle pipe\n");
            exit(2);
        }
    }

    /*In seguito il padre può generare i figli tramite fork*/

    /*CODICE CREAZIONE FIGLI*/
    for(int i = 0; i < N; i++){

        /*Creo figlio memorizzandone il pid*/
        if((pid[i] = fork()) < 0){
            printf("Errore nella creazione del figlio %d\n", i);
            exit(4);
        }

        if(pid[i] == 0){ /* CODICE FIGLIO*/
            
            /*Per prima cosa chiudo pipe inutilizzate*/
            for(int j = 0; j < N; j++){
                if(j != i)
                    close(pipes[j][1]); /*Ogni figlio chiude tutti i lati scrittura tranne il proprio(a destra)*/
                
                if(i == 0 || j != i-1)
                    close(pipes[j][0]); /*Ogni figlio chiude tutti i lati lettura tranne il proprio(a sinistra)*/
            }

            /*Apro qui file in modo che ogni figlio abbia il proprio fd con cui accedere al file*/
            if((fd =open(F, O_RDONLY)) < 0){
                printf("Errore nell'apertura del file %s in lettura, verificare diritti e/o esistenza\n", argv[1]);
                exit(-1);
            }

            if(i > 0){ /*I figli successivi al primo devono leggere l'array di struct da figlio precedente*/
                if(read(pipes[i-1][0], occorrenze, sizeof(occorrenze)) != sizeof(occorrenze)){
                    printf("Errore lettura da pipe[%d]\n", i-1);
                    exit(-1);
                }
            }

            /*Setto i valori dell'elemento corrispondente nell'array*/
            occorrenze[i].v1 = 'a'+i; /*In questo modo il carattere scala avanti in base al figlio*/
            occorrenze[i].v2 = 0; /*Inizializzo contatore occorrenze*/

            /*Leggo un carattere per volta e conto occorrenze*/
            while(read(fd, &carLetto, 1)){
                if(carLetto == 'a'+i){
                    occorrenze[i].v2++;
                }
            }

            /*Scrivo su pipe l'array*/
            write(pipes[i][1], occorrenze, sizeof(occorrenze));
            exit(carLetto); /*Figlio ritorna ultimo carattere letto*/
        } 
    } /*FINE GENERAZIONE FIGLI*/

    /*CODICE PADRE*/
    sleep(3);
    /*Per prima cosa il padre chiude le pipe inutilizzate*/
    for(int i = 0; i < N-1; i++){
        close(pipes[i][0]); /* Chiudo completamente tutte le pipe tranne l'ultima*/
        close(pipes[i][1]);
    }
    close(pipes[N-1][1]); /*Dell'ultima chiudo solo lato scrittura*/

    /*Leggo array di strutture da pipe*/
    if(read(pipes[N-1][0], occorrenze, sizeof(occorrenze)) != sizeof(occorrenze)){
        printf("Errore nella lettura da pipe del padre\n");
        exit(5);
    }

    /*Effettuo ordinamento array di struct e ordino allo stesso modo i pid per mantenere corrispondenza integra*/
    v_occ tmp;
    for(int i = 0; i < N-1; i++){
        for(int j = i+1; j < N; j++)
            if(occorrenze[i].v2 > occorrenze[j].v2){ /*Se precedente maggiore di un elemento dopo, scambio*/
                tmp = occorrenze[i];
                occorrenze[i] = occorrenze[j];
                occorrenze[j] = tmp;
            }
    }

    /*Fase di stampa*/
    printf("\nELENCO DATI OCCORRENZE SU FILE '%s' in ordine crescente:\n", argv[1]);
    for(int i = 0; i < N; i++){
        printf("Carattere %c: %ld occorrenze\n",occorrenze[i].v1, occorrenze[i].v2);
        printf("-> Calcolate da figlio con PID = %d e indice %d\n", pid[occorrenze[i].v1 - 'a'], occorrenze[i].v1 - 'a');
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
            printf("Il figlio di pid = %d ha ritornato %d (se N-15 problemi), ovvero il carattere '%c'\n", pidFiglio, ritorno, (char)ritorno);
        }
    }/*FINE ATTESA FIGLI*/

    exit(0);
}