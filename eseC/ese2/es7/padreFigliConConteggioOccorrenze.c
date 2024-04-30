/* FILE COPIATO DA: status1.c */
#include <stdio.h> //Standard I/O: printf, BUFSIZ
#include <fcntl.h> //File CoNTrol: open, O_RDONLY, O_WRONLY, O_RDWR
#include <stdlib.h> //STanDard LIBrary: exit, malloc, calloc, free, atoi
#include <unistd.h> //UNIx Standard: read, write, close, SEEK_SET, SEEK_CUR, SEEK_END
#include <string.h> //STRING: strlen, strcpy, strcat
#include <sys/stat.h> //SYStem STATus: stat, fstat, S_IFMT, S_IFDIR, S_IFREG
#include <sys/wait.h> //SYStem WAIT: wait
#include <errno.h> //errno


int main (int argc, char* argv[])
{
    int status, pidFiglio, ritorno; /* variabili per valori di ritorno wait */

    int fd; /* variabili per read() */
    char buffer;  

    /* controllo numero parametri */
    if(argc < 4)
    {   /* num parametri errato, deve essere esattamente 1 */
        printf("Non sono stati inseriti almeno 3 parametri!\n");
        printf("USAGE: ./main file1 file2 [...filen] carattere\n");
        exit(1);
    }
    /*numero parametri corretto, proseguo*/
    
    /* controllo che ultimo parametro sia un carattere */
    if(strlen(argv[argc - 1]) != 1){
        printf("Errore: l'ultimo parametro non è un carattere!\n");
        printf("USAGE: ./main file1 file2 [...filen] carattere\n");
        exit(2);
    }
    char Cx = argv[argc - 1][0];

    /* PARAMETRI VALIDI E FILE APERTI IN LETTURA */
    printf("DEBUG: parametri corretti\n\n");

    int contaOccorrenze;
    /* Creazione N figli e interazione con file da parte di ognuno di essi */
    for(int i = 0; i < argc - 2; i++)
    {   
        contaOccorrenze = 0; //resetto per ogni figlio
        if((pidFiglio = fork()) < 0){
            perror("Errore fork");
            exit(4);
        }

        if(pidFiglio == 0)
        {   /* codice figlio */

            if((fd = open(argv[i+1], O_RDONLY)) < 0){
                printf("Errore: non è stato possibile aprire il file %s", argv[i+1]);
                exit(-1);
            }

            /* leggo un carattere per volta e verifico occorrenza*/

            while((read(fd, &buffer, 1)) > 0){ //se non viene letto esattamente 1 carattere
                if(buffer == Cx)
                    contaOccorrenze++;
            }      

            if(contaOccorrenze >= 255) //non devono essere oltre 255
                exit(-1);
            exit(contaOccorrenze); //ogni figlio ritorna le occorrenze trovate
        }
    }

    for(int i = 0; i < argc - 2; i++){
        if((pidFiglio = wait(&status)) < 0){ /* errore wait termina */
            printf("Errore nella wait\n");
            exit(4);
        }

        /* wait con insuccesso, stampo pid, indice ordine e indico terminazione anomala*/
        if((status & 0xFF) != 0){
            printf("Il figlio di pid = %d è terminato in modo anomalo\n", pidFiglio);
        }
        else{
            /* wait con successo, stampo pid, indice e valore di ritorno*/
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha contato %d occorrenze!\n", pidFiglio, ritorno);
        }
    }

	exit(0);
}
