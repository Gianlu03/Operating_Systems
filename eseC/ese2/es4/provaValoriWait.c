/* FILE COPIATO DA: status1.c */
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<string.h>
#include<errno.h>
#include<time.h>

int main ()
{
     int pid; 				/* pid per fork */
     int pidFiglio, status, ritorno;    /* per wait padre */
    int valore; /* valore che legge il figlio */

	if ((pid = fork()) < 0)
	{ 	/* fork fallita */
		printf("Errore in fork\n");
		exit(1);
	}

	if (pid == 0)
	{ 	/* figlio */
		printf("Esecuzione del figlio\n");
		printf("Inserire un numero intero:\n");
        scanf("%d", &valore);

        if(valore > 255 || valore < 0)
            printf("VALORE verrà troncato(PERCHE' SONO RIPORTATI SOLO 8 BIT)\n");
        else
            printf("VALORE NON verrà troncato\n");

		exit(valore);	/* si torna al padre il valore letto in input*/
	}

	/* padre */
	printf("Generato figlio con PID = %d\n", pid);

	/* il padre aspetta il figlio in questo caso interessandosi del valore della exit del figlio */
	if ((pidFiglio=wait(&status)) < 0)
	{
		printf("Errore in wait\n");
		exit(2);
	}

	if (pid == pidFiglio)
        	printf("Terminato figlio con PID = %d\n", pidFiglio);
	else
        {       /* problemi */
                printf("Il pid della wait non corrisponde al pid della fork!\n");
                exit(3);
        }

	if ((status & 0xFF) != 0)
    		printf("Figlio terminato in modo involontario (cioe' anomalo)\n");
    	else
    	{
    		/* selezione del byte "alto" */
    		ritorno = status >> 8;
    		ritorno &= 0xFF;
		/* o anche direttamente:
		ritorno=(int)((status >> 8) & 0xFF);  */
    		printf("Per il figlio %d lo stato di EXIT e` %d\n", pid, ritorno);
    	}

	exit(0);
}

/*
    NOTA BENE
    TRAMITE WAIT NON È POSSIBILE RITORNARE UN INTERO DA PIÙ DI 8 BIT
    NE VENGONO RITORNATI 16 E METÀ SONO SETTATI A 0

    QUINDI I NUMERI SOTTO 0 E OLTRE 255 VENGONO TRONCATI, RIMANENDO TRA 0 E 255 COME
    SE FOSSE UNA STRUTTURA CIRCOLARE   1 2 3 ... 255 1 2 3 ... 255 1 ...
*/