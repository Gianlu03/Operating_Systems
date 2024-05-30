#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve la parte in shell del 18 gennaio 2017
#18Gen17 G DR

#Effettuo controllo su numero parametri: esattamente 2
if test $# -ne 2
then
	echo "Errore: Non sono stati inseriti 2 parametri ma $#"
	echo "Usage: $0 GerAssoluto Dirsemplice \n"
	exit 1
fi

echo "DEBUG: Inseriti 2 parametri, proseguo esecuzione \n"


#Effettuo controllo sul primo parametro: deve essere nome assoluto (di G)
#e deve essere directory traversabile

case $1 in
/*)	
	if test -d $1 -a -x $1
	then 	
		echo "DEBUG: Primo parametro è directory traversabile, proseguo esecuzione \n"
	else
		echo "Errore: Primo parametro non directory o non traversabile"
		exit 2
	fi
;;
*)	echo "Errore: Il primo parametro inserito non è un nome assoluto"
	exit 3
;;
esac

#Effettuo controllo sul secondo parametro: deve essere nome rel. semplice(DR)
#Non devo verificare se è directory perchè non è detto sia già nella dir #corrente

case $2 in
*/*)
	echo "Errore: Il secondo parametro non è un nome relativo semplice"
	exit 4
;;
*)	
	echo "DEBUG: Primo parametro è nome relativo semplice, proseguo esecuzione \n"
;;
esac

#Una volta verificata la validità dei controlli modifico ed esporto
#$PATH così che venga copiata dai processi shell figli
PATH=`pwd`:$PATH
export PATH

#Creo file temporaneo in temp da passare al file comandi ricorsivo
#(Alla fine verrà rimosso)

> /tmp/tmp$$ #con $$ viene posto il codice processo in fondo per riconoscerlo

#Ora posso richiamare la componente ricorsiva del codice

sh FCR.sh $* /tmp/tmp$$ #usa gli stessi parametri più il file temporaneo


#Stampo anche se non richiesto numero e nomi file trovati
echo "I file individuati sono:\n"
cat /tmp/tmp$$

echo "DEBUG: Invoco componente C"

#Rimuovo file temporaneo
rm /tmp/tmp$$








