#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int main(int argc, char** argv){

    char* usage = "USAGE: ./main nomefile carattere";

    

    //Controllo numero parametri stretto = 3 (1 del nome programma, 3 parametro effettivo)
    if(argc != 3) {
        printf("Errore. è necessario passare esattamente 2 parametri\n");
        printf("%s\n", usage);
        exit(1);
    }

    int F;
    //Per il primo parametro controllo se riesco ad aprirlo in lettura
    if((F = open(argv[1], O_RDONLY)) < 0){
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
    int n = atoi(argv[2]);  //prendo secondo parametro in variabile con nome richiesto

    char* buffer = malloc(sizeof(char) * n);
    int nread;
    printf("I caratteri multipli sono i seguenti:\n");
    while((nread = read(F, buffer, n)) == n){  //leggo n caratteri alla volta e stampo n-esimo letto
        printf("Il carattere multiplo %d-esimo nel file %s è: %c\n", SEEK_CUR, argv[1], buffer[n-1]);
    }                                                       //mostro posizione attuale, in quale file e il carattere trovato

    printf("\nFine esecuzione\n");  


}