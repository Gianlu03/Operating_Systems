#!/bin/sh
#FCP.sh
#File Comandi Ricorsivo che risolve esame shell del 12 lug 2023

#PARAMETRI
# $1 = gerarchia  $2 = interoX  $3 = file temporaneo   

#mi posiziono nella gerarchia passata
cd $1


#Ciclo per scorrere il contenuto della directory
for F in *
do
    if test -f $F -a -r $F #Controllo tra i file leggibili
    then
        nRighe=`wc -l < $F`
        if test $nRighe -lt $2 #se rispetta numero righe massimo
        then
            echo "Trovato valido `pwd`/$F"
            echo `pwd`/$F >> $3  #copio nel file temporaneo
        fi
    else
        if test -d $F -a -x $F  #Se directory continuo ricorsione
        then
            $0 `pwd`/$F $2 $3 #Richiamo file ricorsivo spostandomi nella sottodirectory
        fi
    fi


done