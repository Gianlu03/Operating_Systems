/* FILE: pipe.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define MSGSIZE 512

int main (int argc, char **argv)
{   
   int pid, nMessaggi, piped[2]; 		/* pid per fork, nMessaggi per indice, piped per pipe */
   /*Allocato per poter usare la calloc e azzerare la stringa*/
   char mess[512];			/* array usato dal figlio per inviare la stringa al padre e array usato dal padre per ricevere stringa inviata dal figlio */
   /* N.B. dato che ogni processo (padre e figlio) avra' la propria AREA DATI il contenuto di questo array sara' PRIVATO di ogni processo e quindi NON serve creare due array distinti per il padre e per il figlio! */
   /* La stessa cosa vale anche per tutte le altre variabili chiaramente! */
   int pidFiglio, status, ritorno;      /* per wait padre */

	if (argc != 2)
	{	printf("Numero dei parametri errato %d: ci vuole un singolo parametro\n", argc);
    		exit(1);
	}

	/* si crea una pipe: si DEVE sempre controllare che la creazione abbia successo!  */
	if (pipe(piped) < 0 )
	{   	printf("Errore creazione pipe\n");
    		exit(2); 
	}

	/* si crea un figlio (controllando sempre!) */
	if ((pid = fork()) < 0)  
	{   	printf("Errore creazione figlio\n");
    		exit(3); 
	}
	if (pid == 0)  
	{   
		/* figlio */
		int fd;	/* variabile presente solo nello stack del figlio; va bene anche definirla nel main */
		close(piped[0]); 	/* il figlio CHIUDE il lato di lettura: sara' quindi lo SCRITTORE della pipe */
		if ((fd = open(argv[1], O_RDONLY)) < 0)		/* apriamo il file */
		{   	printf("Errore in apertura file %s\n", argv[1]);
            		exit(-1); /* torniamo al padre un -1 che sara' interpretato come 255 e quindi identificato come errore */
		}

		printf("DEBUG-Figlio %d sta per iniziare a scrivere una serie di messaggi, ognuno al massimo di lunghezza %d, sulla pipe dopo averli letti dal file passato come parametro\n", getpid(), MSGSIZE);
		
        int contaCaratteri = 0;
        nMessaggi = 0;

		while (read(fd, &mess[contaCaratteri], 1)) /* il contenuto del file e' tale che in mess ci saranno 4 caratteri e il terminatore di linea */
		{   
            contaCaratteri++; //ad ogni lettura conto un carattere    
            /* Una volta letto il terminatore di riga pongo il terminatore di stringa 
            e scrivo su pipe la linea*/
            if(mess[contaCaratteri-1] == '\n'){
                mess[contaCaratteri-1] = '\0'; 
                nMessaggi++; /* incrementiamo il contatore dei messaggi inviati*/
                write(piped[1], &contaCaratteri, sizeof(int));
                write(piped[1], mess, contaCaratteri); /* scrivo messaggio su pipe */
                contaCaratteri = 0;
            }
		}
		printf("DEBUG-Figlio %d scritto %d messaggi sulla pipe\n", getpid(), nMessaggi);
		exit(nMessaggi);	/* figlio deve tornare al padre il numero di linee lette che corrisponde al numero di stringhe mandate al padre, supposto < 255! */
	}

	/* padre */
	close(piped[1]); /* il padre CHIUDE il lato di scrittura: sara' quindi il LETTORE della pipe */		
	printf("DEBUG-Padre %d sta per iniziare a leggere i messaggi dalla pipe\n", getpid());
	nMessaggi=0; /* il padre inizializza la sua variabile nMessaggi per verificare quanti messaggi ha mandato il figlio */
	
    int lunghezzaMessaggio;

    while (read(piped[0], &lunghezzaMessaggio, sizeof(int)))  /* questo ciclo avra' termine appena il figlio terminera' dato che la read senza piu' scrittore tornera' 0! */
	{ 	
        /*Leggo messaggio*/
        read(piped[0], mess, lunghezzaMessaggio);

        /*Stampo messaggio ricevuto*/
        printf("%d: %s\n", nMessaggi, mess);
        nMessaggi++;
	}

	printf("DEBUG-Padre %d ha letto %d messaggi dalla pipe\n", getpid(), nMessaggi);

	/* padre aspetta il figlio */
	if ((pidFiglio=wait(&status)) < 0)
	{
      		printf("Errore wait\n");
      		exit(4);
	}
	if ((status & 0xFF) != 0)
        	printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
	else
	{
       		ritorno=(int)((status >> 8) & 0xFF);
       		printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
	}

	exit(0);
}