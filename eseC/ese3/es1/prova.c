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
    char input;
    printf("Eseguire richiamata ricorsiva? (0 = no)\n");
    scanf("%c", &input);

    if(input != '0'){
        execl("/home/gian/Desktop/Operating_Systems/eseC/ese3/es1/main", "main", (char*)0);

        printf("Errore Exec\n");
        exit(1);
    }

	exit(0);
}
