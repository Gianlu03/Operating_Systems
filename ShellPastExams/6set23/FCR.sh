#!/bin/sh
#FCP.sh
#File Comandi Ricorsivo che risolve esame shell del 6 settembre 23

#PARAMETRI
# $1 = gerarchia   $2 = M  successivi = stringhe   

#mi posiziono nella gerarchia passata
cd $1
M=$2 #memorizzo M per poi shiftare 2 volte
shift 2
#ora rimangono solo le stringhe tra i parametri

#Scorro il contenuto per cercare file con nomi uguali ai parametri
cont=0 #contatore per contare quanti file trovo
files= #variabile "lista" per contenere i file validi da passare al C

for F in *  #Per ogni elemento in directory
do
    if test -f $F -a -r $F  #Se file leggibile
    then
        for i in $*
        do
            if test $i = $F  #Se il file ha nome uguale alla stringa 
            then
                files="$files $i" #memorizzo file valido
                cont=`expr $cont + 1` #aumento contatore
            fi
        done
    fi
done

if test $cont -ge $M  #Verifico se ho trovato almeno M file validi nella directory
then
    echo "Directory valida: `pwd`"  #se li ho trovati stampo su stdout directory
    echo "Main.c $files"  #e richiamo file C
fi

#Continuo esplorazione ricorsiva
for F in *
do
    if test -d $F -a -x $F  #Se trovo sottodirectory traversabile
    then
        $0 `pwd`/$F $M $*   #richiamo ricorsione con sottodirectory
    fi
done

