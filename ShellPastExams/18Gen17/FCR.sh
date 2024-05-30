#!/bin/sh
#FCR.sh
#File Comandi Ricorsivo per risolvere simulazione del 18 gennaio 2017

#Mi posiziono nella gerarchia fornita
cd $1

#Uso una variabile "Booleana" per stampare una directory quando impostata a true, ovvero se è individuato un file come richiesto
trovato=false

case $1 in
*/$2)   #il controllo dei file viene effettuato solo nelle dir con il nome corretto
	for i in *
	do
		if test -f $i #verifico nei file
		then
		#Verifico occorrenza carattere numerico
			if grep [0-9] $i > /dev/null 2>&1 #output e errore di grep buttati
			then
				#Salvo il nome assoluto del file nel temporaneo
				echo `pwd`/$i >> $3
				#Setto a true il "booleano"
				trovato=true
			fi
		fi
	done
;;
esac

#Ora, se nella directory con nome corretto ho trovato un file come richiesto, stampo
if test $trovato = true   
then
	echo Ho trovato una directory che soddisfa le specifiche: `pwd`
fi

#Ora richiamo nuovamente ricorsione per andare più in profondità nella gerarchia
for i in *
do
	if test -d $i -a -x $i
	then
		FCR.sh `pwd`/$i $2 $3  #Cambio solo radice da cui continuare ricerca ad albero
	fi
done






