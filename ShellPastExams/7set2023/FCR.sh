#!/bin/sh
#FCP.sh
#File Comandi Ricorsivo che risolve esame shell del 7 settembre 2022

#PARAMETRI
# $1 = gerarchia   $2 = X  $3 = file temporaneo   

#mi posiziono nella gerarchia passata
cd $1
echo "$2"
#esploro contenuto directory 
files=  #Variabile per contenere file validi da inviare a script C
for F in *
do
    if test -f $F -a -r $F  #Se trovo file leggibile
    then
        nCaratteri=`wc -c < $F`
        echo "nCaratteri=$nCaratteri"
        if test $nCaratteri -eq $2  #Verifico se ha il numero di caratteri richiesto
        then
            files="$files $F" #compongo lista file validi
            echo `pwd`/$F >> $3  #Copio nel file temporaneo il nome assoluto
        fi
    fi
done

if test "$files"  #Se la directory contiene almeno un file valido richiamo C
then
    echo "main.c $files"
fi



#la continuazione della ricorsione è svolta dopo per visualizzare per livelli il contenuto
for F in *
do
    if test -d $F -a -x $F
    then
        $0 `pwd`/$F $2 $3  #Richiamo file ricorsivo scendendo nella gerarchia
    fi
done
