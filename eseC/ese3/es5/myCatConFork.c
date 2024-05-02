/* FILE: myGrepConFork.c */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main (int argc, char** argv)
{
    int pid;        			/* per fork */
    int pidFiglio, status, ritorno;     /* per wait padre */

    /* controlliamo che si passi esattamente un parametro*/
	if (argc-1 != 1)
	{
       		printf("Errore nel numero di parametri che deve essere 1 solo(nome file");
       		exit(1);	
	}



	/* generiamo un processo figlio dato che stiamo simulando di essere il processo di shell! */
        if ((pid = fork()) < 0)
	{	/* fork fallita */
		printf("Errore in fork\n");
		exit(2);
	}

	if (pid == 0)
	{ 	/* figlio */
		printf("Esecuzione di mycat da parte del figlio con pid %d\n", getpid());
	/* ridirezionamo lo standard output su /dev/null perche' ci interessa solo se il comando grep ha successo o meno */
		close(0);
        if(open(argv[1], O_RDONLY) < 0)
            exit(-2); /* errore 2 = 254*/
		execl("/home/gian/Desktop/Operating_Systems/eseC/ese3/es5/mycat", "mycat", (char *)0); //richiamo mycat senza parametri perchè il file è aperto in 0

 		/* si esegue l'istruzione seguente SOLO in caso di fallimento della execlp */
		/* ATTENZIONE SE LA EXEC FALLISCE NON HA SENSO FARE printf("Errore in execlp\n"); DATO CHE LO STANDARD OUTPUT E' RIDIRETTO SU /dev/null */
        exit(-1); /* torniamo al padre un -1 che sara' interpretato come 255 e quindi identificato come errore */
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
        printf("Il figlio con pid=%d ha ritornato %d (se 254-5 problemi!)\n", pidFiglio, ritorno);
		if (ritorno == 0)
       			printf("il figlio %d ha ritornato %d\n", pidFiglio, ritorno);
	}	
	exit(0);
}