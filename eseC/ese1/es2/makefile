CC=gcc
cflags=-Wall -std=c17

all:main 

main: parametri2.o	#l'eseguibile dipende dai .o
	$(CC) $(cflags) -o $@ $<

parametri2.o: parametri2.c   #i .o dipendono dal proprio .c  con -c
	$(CC) $(cflags) -c $<

clean:  #elimina i .o per ricostriuire da capo
	rm -rf *.o