#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int main  (int argc, char **argv){
    
    char* usage = "USAGE: ./main file interoPositivo";
    int fd;

    //Per il primo parametro controllo se riesco ad aprirlo in lettura
    if((fd = open(argv[1], O_RDONLY)) < 0){
        printf("Errore. il primo parametro non è un file leggible\n");
        printf("%s\n", usage);
        exit(1);
    }

    //Controllo numero intero -> verifico se atoi ritorna <= 0 (comprende sia zero che errore conversione)
    if(atoi(argv[2]) <= 0){
        printf("Errore. il secondo parametro non è un intero positivo\n");
        printf("%s\n", usage);
        exit(2);
    }

    int n = atoi(argv[2]);

    int nlinea = 1;
    int nread;
    char buffer;
    printf("Reperimento linea richiesta...\n");
    while ((nread = read(fd, &buffer, 1)) == 1){
        if(nlinea == n)
            printf("%c", buffer);
        
        if(nlinea > n)
            break;

        if(buffer == '\n')
            nlinea++;
    }

    if(nlinea < n)
        printf("Non è stata trovata la linea richiesta\n");

    printf("\nFINE PROGRAMMA\n");
}