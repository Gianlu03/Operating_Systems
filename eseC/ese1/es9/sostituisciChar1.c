#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int main(int argc, char** argv){

    char* usage = "USAGE: ./main nomefile carattere";

    //Controllo numero parametri stretto = 3 (1 del nome programma, 3 parametro effettivo)
    if(argc != 4) {
        printf("Errore. è necessario passare esattamente 3 parametri\n");
        printf("%s\n", usage);
        exit(1);
    }

    int F;
    //Per il primo parametro controllo se riesco ad aprirlo in lettura
    if((F = open(argv[1], O_RDWR)) < 0){ //sia lettura che scrittura
        printf("Errore. il primo parametro non è un file leggible\n");
        printf("%s\n", usage);
        exit(2);
    }


    //Per verificare carattere uso strlen  -> includi string.h
    if(strlen(argv[2]) != 1){
        printf("Errore. il secondo parametro non è un carattere\n");
        printf("%s\n", usage);
        exit(3);
    }
    char Cx = argv[2][0];  //prendo in variabile richiesta il carattere parametro

    if(strlen(argv[3]) != 1){
        printf("Errore. il terzo parametro non è un carattere\n");
        printf("%s\n", usage);
        exit(4);
    }
    char change = argv[3][0];  //prendo in variabile richiesta il carattere parametro

    char buffer;
    int nread;
    while((nread = read(F, &buffer, 1)) == 1) { //leggo un singolo carattere per volta
        if(buffer == Cx){
            buffer = change;
            lseek(F, -1, SEEK_CUR); //torno indietro di un carattere perchè lettura mi ha spostato in avanti
            write(F, &buffer, 1); //scrivo carattere vuoto al posto di corrispondenza
        }
    }
    
    printf("TERMINE PROGRAMMA\n\n");

    exit(0);
}