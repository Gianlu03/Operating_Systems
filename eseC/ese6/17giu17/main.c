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

/*Definisco la struct richiesta*/
typedef struct{

}nomestruct;

int main(int argc, char** argv){
    /*====================================VARIABILI================================================*/
    
    int pid; /*Variabile per ritorno fork*/
    int pidFiglio, ritorno, status; /*Variabili per wait*/
    pipe_t *pipes; /*array di pipe per comunicazione tra processi*/
    int fd; /*variabile per contenere file descriptor e interagire con file*/
    char token = 't'; /*Carattere da fornire come informazione minima nella pipe per continuare/fermare algoritmo*/
    long int occorrenze = 0; /*variabile per contare occorrenze*/
    char carLetto; /*Variabile di buffer che contiene carattere letto*/

    char* USAGE = "USAGE: ./main file1 [...filen] CarattereCx\n"; /*Stringa USAGE da stampare in caso di parametri errati*/
    /*==============================================================================================*/

    /*Controllo numero parametri*/
    if (argc < 3)
    {
        printf("Errore: hai inserito %d parametri, necessari almeno 3\n", argc - 1);
        printf("%s\n", USAGE);
        exit(1);
    }
    int N = argc-2;


    exit(0);
}


