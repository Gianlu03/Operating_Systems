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
    int trovata = 0;
    char *buffer = malloc(sizeof(char) * 256);
    int lunghezzaLinea=0;
    printf("linee lunghe %d caratteri:\n", n);
    while ((nread = read(fd, &buffer[lunghezzaLinea], 1)) == 1){
        lunghezzaLinea++; //per ogni carattere letto aumenta la lunghezza

        if(buffer[lunghezzaLinea-1] == '\n'){
            if(lunghezzaLinea == n) // se la linea è lunga n la stampo
            {
                trovata = 1;
                printf("-linea %d:\n   %s", nlinea, buffer);

            }
            nlinea++; //passo alla linea dopo
            lunghezzaLinea=0; //ricomincio a contare da 0 nella linea successiva
            buffer = calloc(256, sizeof(char));  //rimetto tutto a caratteri vuoti il buffer
        }        
    }

    if(trovata == 0)
        printf("Non sono state trovate righe di lunghezza %d", n);

    printf("\nFINE PROGRAMMA\n");
}