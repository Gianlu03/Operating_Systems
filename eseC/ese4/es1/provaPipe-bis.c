/* FILE COPIATO DA: status1.c */
#include <stdio.h> //Standard I/O: printf, BUFSIZ
#include <fcntl.h> //File CoNTrol: open, O_RDONLY, O_WRONLY, O_RDWR
#include <stdlib.h> //STanDard LIBrary: exit, malloc, calloc, free, atoi
#include <unistd.h> //UNIx Standard: read, write, close, SEEK_SET, SEEK_CUR, SEEK_END
#include <string.h> //STRING: strlen, strcpy, strcat
#include <sys/stat.h> //SYStem STATus: stat, fstat, S_IFMT, S_IFDIR, S_IFREG
#include <sys/wait.h> //SYStem WAIT: wait
#include <errno.h> //errno
#include <time.h> //srand(time(NULL))


int main (int argc, char* argv[])
{
    int fd1, fd2;  /* variabili per file descriptor dei 2 file*/
    int piped[2]; /* Array per contenere i 2 file descriptor della pipe*/
    char USAGE[] = "USAGE: ./main filename1 filename2\n"; /* stringa da stampare con errore dell'utente */

    int N = argc - 1;

    /* Controllo numero di parametri: devono essere esattamente 2*/
    if(N != 2){
        printf("Errore: Sono stati inseriti %d parametri: necessari esattamente 2 parametri\n", N);
        printf("%s", USAGE);
        exit(1);
    }

    /* Verifico corretta apertura primo file*/
    if((fd1 = open(argv[1], O_RDONLY)) < 0){
        printf("Errore: non è stato possibile aprire in apertura %s, verificare l'esistenza o i diritti di tale file", argv[1]);
        exit(2);
    }

    /* Verifico corretta apertura secondo file*/
    if((fd2 = open(argv[2], O_RDONLY)) < 0){
        printf("Errore: non è stato possibile aprire in apertura %s, verificare l'esistenza o i diritti di tale file", argv[2]);
        exit(3);
    }

    /* Chiudo primo file */
    close(fd1);

    if(pipe(piped) < 0){
        printf("Errore nella creazione della pipe\n");
        exit(4);
    }

    printf("I valori dei file descriptor della pipe sono:\n");
    printf("piped[0]: %d\n", piped[0]);
    printf("piped[1]: %d\n", piped[1]);

    printf("TERMINE PROGRAMMA\n");

	exit(0);
}
