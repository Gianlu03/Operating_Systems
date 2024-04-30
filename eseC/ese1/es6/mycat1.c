#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char **argv)
{ 
    int nread; /* valore ritorno read */
    char buffer[BUFSIZ]; /* usato per i caratteri */
    int fd = 0; 
    
    int i = 0;
    if(argc >= 2){
        for(i = 1; i < argc; i++){
            if((fd = open(argv[i], O_RDONLY)) < 0){
                printf("Errore: non è stato possibile aprire il lettura il file %s", argv[i]);
                exit(1);
            }   

            while((nread = read(fd, buffer, BUFSIZ)) > 0)
                write(1, buffer, nread); //1 perchè voglio scrivere su standard output
        }
    }
    printf("\n");

    exit(0);
}
