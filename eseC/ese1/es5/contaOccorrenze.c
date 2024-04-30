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

    int fd;
    //Per il primo parametro controllo se riesco ad aprirlo in lettura
    if((fd = open(argv[1], O_RDONLY)) < 0){
        printf("Errore. il primo parametro non è un file leggible\n");
        printf("%s\n", usage);
        exit(2);
    }

    //int N = atoi(argv[2]);

    //Per verificare carattere uso strlen  -> includi string.h
    if(strlen(argv[2]) != 1){
        printf("Errore. il secondo parametro non è un carattere\n");
        printf("%s\n", usage);
        exit(3);
    }

    printf("\nIl nome dell'eseguibile è %s", argv[0]);
    printf("\n");
    printf("Il numero di parametri è %d", argc - 1); //-1 perchè non conto nome eseguibile
    printf("\n\n");

    printf("Il primo parametro(file) è %s", argv[1]);
    printf("\n");
    printf("Il secondo parametro (carattere) è %s", argv[2]);
    printf("\n");

    char Cx = argv[2][0];  //memorizzo con nome desiderato il carattere
    int nread, n = 1; //variabili per read
    char buffer; //preparo buffer per contenere un carattere per volta

    int contaOccorrenze = 0; //contatore per contare le occorrenze del carattere passato nel file
    while((nread = read(fd, &buffer, n)) == n){
        if(buffer == Cx){
            contaOccorrenze++;
        }
    }

    printf("\nAll'intero del file sono state trovate %d occorrenze del carattere %c\n", contaOccorrenze, Cx);

    printf("TERMINE PROGRAMMA\n\n");

    exit(0);
}