# Nella variabile CC indico il compilatore da usare
CC=gcc
#In cflags indico i flag, ovvero i -* per il comando
cflags=-Wall -std=c11


#target		#Dependencies
main: parametri1.c   #con -o viene creato un eseguibile con il primo nome e compilati tutti i seguenti
	$(CC) $(cflags) -o $@ $<
#con $() sostituisco le variabili, quindi sto chiamando gcc -Wall

#$@ recupera il target, $< la prima dipendenzA(a sinistra)

#Per pattern matching:  % al posto di *  -> %: %.c  per creare in automatico la dipendenza per tutti i target

#gcc -c effettua solo compilazione dei file indicati, 
#gcc -o effettua compilazione ma anche linking andando a creare il file oggetto .o