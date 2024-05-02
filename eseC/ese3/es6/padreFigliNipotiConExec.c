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
    int pid, pid2;        			/* per fork */
    int pidFiglio, status, ritorno;     /* per wait padre */
    int pidNipote, status2, ritorno2;   /* per wait figlio*/

    /* controlliamo che si passino almeno 3 parametri */ 
	if (argc-1 < 3)
	{
        printf("Errore nel numero di parametri che devono essere almeno 3(nomi dei file)");
        exit(1);	
	}

	/* genero un processo figlio per ogni file fornito come parametro */
    for(int i = 0; i < argc-1; i++){
        if ((pid = fork()) < 0)
        {	/* fork fallita */
            printf("Errore in fork\n");
            exit(2);
        }

        if (pid == 0)
        { 	/* figlio */
            char* FOut = malloc(sizeof(char) * 255);
            strcpy(FOut, argv[i+1]);
            strcat(FOut, ".sort");
            if ((pid2 = fork()) < 0)
            {	/* fork fallita */
                printf("Errore in fork nipoti\n");
                exit(2);
            }

            if(pid2 == 0){
                /* codice nipote */
                if(creat(FOut, PERM) < 0){
                    printf("Errore creat nipote %d di figlio %d", getpid(), getppid());
                    exit(-1);
                }
                //il nipote esegue il sort sul file parametro, scrivendolo su fout
                //ridirigo standard output su FOut, così che la sort scriva qui

                close(0); //ridirigo standard input su file passato come parametro
                if(open(argv[i+1], O_RDONLY) < 0){
                    printf("Errore File input nipote %d di figlio %d\n", getpid(), getppid());
                    exit(-1);
                }
                
                close(1); //ridirigo standard output su file di output come richiesto
                if(open(FOut, O_WRONLY | O_TRUNC) < 0){
		    printf("Errore File output nipote %d di figlio %d\n", getpid(), getppid());
                    exit(-1);
                }

                execlp("sort", "sort", (char*)0);
                
                //qui arriva solo con errore di exec
                printf("Errore execlp\n");
                exit(-1);
            }

            if ((pidNipote=wait(&status2)) < 0)
            {
                    printf("Errore wait figlio\n");
                    exit(3);
            }
            if ((status2 & 0xFF) != 0)
                    printf("Nipote con pid %d dal figlio %d terminato in modo anomalo\n", getpid(), pidNipote);
            else
            {	
                ritorno2=(int)((status2 >> 8) & 0xFF);
                if(ritorno2 == 255)
                    printf("Nipote di figlio %d ha ritornato -1\n", getpid());
                free(FOut);
                exit(ritorno2);
            }
        }

        /* padre aspetta subito il figlio appunto perche' deve simulare la shell e la esecuzione in foreground! */		
        if ((pidFiglio=wait(&status)) < 0)
        {
                printf("Errore wait\n");
                exit(3);
        }
        if ((status & 0xFF) != 0)
                printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else
        {	
            ritorno=(int)((status >> 8) & 0xFF);
            printf("Figlio con pid %d ha ritornato %d(255 problemi!)\n", pidFiglio, ritorno);
        }	
    }
    
	exit(0);
}
