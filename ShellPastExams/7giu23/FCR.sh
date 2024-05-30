#!/bin/sh
#FCR.sh
#File Comandi ricorsivo che risolve la parte in shell del 7 giugno 2023

#PARAMETRI
# $1 = gerarchia  $2 = interoX  $3 = contalivelli  $4 = file temporaneo

#Mi posiziono nella gerarchia ricevuta
echo "Ho ricevuto $1"
cd $1

#Se raggiungo il livello richiesto, salvo directory e chiamo file C
if test $3 -eq $2
then
    pwd >> $4  #Se il livello a cui mi trovo coincide con X scrivo sul file il path assoluto
    files= #variabile per memorizzare file leggibili non vuoti
    for F in *
    do
        if test -f $F -a -r $F   #un file è valido se leggibile e non vuoto
        then
            ncaratteri=`wc -c < $F 2>/dev/null`
            if test $ncaratteri -gt 0
            then
                files="$files `pwd`/$F"
            fi
            
        fi
    done
    #Una volta qui ho memorizzato tutti i file validi
    echo "Richamo file C con parametri $files"
else
    #Scorro il contenuto per individuare subdirectory per proseguire ricerca(non svolto oltre il livello x)
    for F in *
    do
        #La ricerca prosegue in directory eseguibili solo se prima del livello richiesto
        #altrimenti non ha senso la ricerca
        if test -d $F -a -x $F -a $3 -lt $2 
        then
            $0 `pwd`/$F $2 `expr $3 + 1` $4  #scendo nella gerarchia e aumenta il conta livello
        fi
    done
fi