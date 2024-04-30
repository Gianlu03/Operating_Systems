#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int main(int argc, char** argv){

    char* usage = "USAGE: ./main nomefile intPosN carattere [...]";

    //Controllo numero parametri stretto = 3 (1 del nome programma, 3 parametro effettivo)
    if(argc != 4) {
        printf("Errore. è necessario passare esattamente 3 parametri\n");
        printf("%s\n", usage);
        exit(1);
    }

    int fd;
    //Per il primo parametro controllo se riesco ad aprirlo in lettura
    if((fd = open(argv[1], O_RDONLY)) < 0){
        printf("Errore. il primo parametro non è un file leggible\n");
        printf("%s\n", usage);
        exit(2);
    }

    //Controllo numero intero -> verifico se atoi ritorna <= 0 (comprende sia zero che errore conversione)
    if(atoi(argv[2]) <= 0){
        printf("Errore. il secondo parametro non è un intero positivo\n");
        printf("%s\n", usage);
        exit(3);
    }

    //int N = atoi(argv[2]);

    //Per verificare carattere uso strlen  -> includi string.h
    if(strlen(argv[3]) != 1){
        printf("Errore. il terzo parametro non è un carattere\n");
        printf("%s\n", usage);
        exit(4);
    }

    printf("\nIl nome dell'eseguibile è %s", argv[0]);
    printf("\n");
    printf("Il numero di parametri è %d", argc - 1); //-1 perchè non conto nome eseguibile
    printf("\n\n");

    printf("Il primo parametro(file) è %s", argv[1]);
    printf("\n");
    printf("Il secondo parametro intero positivo è %s", argv[2]);
    printf("\n");
    printf("Il terzo parametro(carattere) è %s", argv[3]);
    printf("\n");


    printf("TERMINE PROGRAMMA\n\n");

    exit(0);
}