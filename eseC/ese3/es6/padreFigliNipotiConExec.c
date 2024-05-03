/* FILE: myGrepConFork.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#define PERM 0644

int main (int argc, char** argv)
{
    int fd;
    int pid;        			/* per fork */
    int pidFiglio, status, ritorno;     /* per wait padre */

    int N = argc - 1; //memorizzo come richiesto numero parametri in variabile N  (senza nome programma)
    /* controlliamo che si passino almeno 3 parametri */ 
	if (N < 3)
	{
        printf("Errore nel numero di parametri che devono essere almeno 3(forniti %d parametri)\n", N);
        exit(1);	
	}

	/* genero un processo figlio per ogni file fornito come parametro */
    for(int i = 0; i < N; i++){
        if ((pid = fork()) < 0)
        {	/* fork fallita */
            printf("Errore in fork\n");
            exit(2);
        }

        if (pid == 0) /* CODICE FIGLIO*/
        { 	
            /* Creo nome file output concatenando .sort*/
            char* FOut = malloc(sizeof(char) * ((strlen(argv[i+1]) + 6)));

            if(FOut == NULL){ /* controllo che malloc sia andata a buon fine*/
                printf("Errore nelle malloc di FOut\n");
                exit(-1);
            }

            strcpy(FOut, argv[i+1]); //Strcpy necessaria per avere una copia separata della stringa
            strcat(FOut, ".sort"); //concateno .sort a FOut

            //Creo file .sort, se già esistente viene azzerato (controllo anche se creato con successo)
            if((fd = creat(FOut, PERM)) < 0){
                printf("Errore creazione di %s\n", FOut);
                exit(-1);
            }
            /*Chiusura file in quanto verrà aperto in seguito in posizione 1 della tabella dei file*/
            close(fd);  /* Se lo si lasciasse occuperebbe 2 righe della tabella */
            
            /*Creo nipote*/
            if ((pid = fork()) < 0)
            {	/* fork fallita */
                printf("Errore in fork per generare nipote\n");
                exit(2);
            }

            if(pid == 0){ /* CODICE NIPOTE */
                /*ridirigo standard input per usare comando filtro*/
                close(0);
                if(open(argv[i+1], O_RDONLY) < 0){ /* Controllo se aperto correttamente*/
                    printf("Errore File %s in lettura \n", argv[i+1]);
                    unlink(FOut); //in caso di problemi elimino file.sort
                    exit(-1);
                }
                
                //ridirigo standard output per scrivere su FOut al posto di linea di comando
                close(1);
                if(open(FOut, O_WRONLY) < 0){ //dopo la creat è sufficiente aprire in lettura, è già troncato
                    printf("Errore file %s in scrittura\n", FOut);
                    unlink(FOut); //in caso di problemi elimino file.sort
                    exit(-1);
                }

                /*Eseguo comando filtro sort*/
                execlp("sort", "sort", (char*)0);
                
                //Solo in caso di errore di Execlp si giunge a questo errore
                perror("Errore Exec del sort nel nipote\n");
                exit(-1);
            }

            /*fine nipote, ora il figlio attende il ritorno del nipote che ha creato */
            if ((pid = wait(&status)) < 0)
            {   /*errore wait*/
                printf("Errore wait(figlio)\n");
                exit(-1);
            }
            if ((status & 0xFF) != 0){ /* terminazione anomala nipote*/
                printf("nipote con pid %d terminato in modo anomalo\n", pid);
                exit(-1);
            }
            else
            {	/* terminazione corretta*/
                ritorno=(int)((status >> 8) & 0xFF);
                exit(ritorno); //il figlio riporta il valore del nipote al padre
            }	
        }  /*FINE CODICE FIGLIO */    
    } /* FINE CICLO CREAZIONE */

    /* FASE DI ATTESA DEL PADRE*/
    for(int i = 0; i < N; i++){
        if ((pidFiglio = wait(&status)) < 0)
        {   /*errore wait*/
            printf("Errore wait padre\n");
            exit(3);
        }
        if ((status & 0xFF) != 0) /* terminazione anomala */
            printf("figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else
        {	/* terminazione corretta*/
            ritorno=(int)((status >> 8) & 0xFF);
            printf("Il figlio di pid = %d ha ritornato %d (255 = problema)\n", pidFiglio, ritorno);
        }	
    } /* FINE CICLO ATTESA PADRE */

	exit(0);
}




/* NOTE:
    TUTTI GLI ERRORI AL DI SOTTO DEL LIVELLO DEL PADRE SONO INDICATI CON -1, COSÌ CHE
    VENGA RITORNATO 255 (PROBLEMA)

    GLI ERRORI NEL PADRE NUMERATI IN MODO INCREMENTALE

    UTILIZZA COMMENTI PER INDICARE A COSA SI RIFERISCONO LE GRAFFE

    EFFETTUARE SEMPRE CONTROLLO MALLOC == NULL

    PER NIPOTI NON E' NECESSARIO PREPARARE ULTERIORI VARIABILI PER RITORNO NIPOTI E FORK

    AGGIUNTO UNLINK PER ELIMINARE FILE .SORT IN CASO DI ERRORI
*/
