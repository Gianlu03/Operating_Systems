#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>

int main(int argc, char** argv){

    char* usage = "USAGE: ./main par1 [...]";

    //Controllo numero parametri lasco, almeno 1 parametro (1 del nome programma, 1 parametro effettivo)
    if(argc < 2) {
        printf("Errore. è necessario passare almeno un parametro\n");
        printf("%s\n", usage);
        exit(1);
    }

    printf("Il nome dell'eseguibile è %s", argv[0]);
    printf("\n");
    printf("Il numero di parametri è %d", argc - 1); //-1 perchè non conto nome eseguibile
    printf("\n");

    printf("Segue la lista dei parametri"); //-1 perchè non conto nome eseguibile
    printf("\n");

    //stampo elenco parametri
    int i = 1;
    for(i = 1; i < argc; i++){
        printf("parametro %d: %s",i , argv[i]);
        printf("\n");
    }

    printf("TERMINE PROGRAMMA\n\n");

    exit(0);
}