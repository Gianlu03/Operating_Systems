#!/bin/sh
#FCP.sh
#File Comandi Ricorsivo che risolve esame del 17 giugno 2020

#PARAMETRI
# $1 = gerarchia   $2 = interoPositivoB 

#mi posiziono nella gerarchia passata
cd $1
B=$2 #memorizzo l'intero positivo con il nome 'B' richiesto

#variabili per indicare se trovate le caratteristiche richieste della directory
subDirTrovata=false
fileValidoTrovato=false

files=  #Variabile "lista" per contenere i file validi

for F in *  #Scorro il contenuto della dir
do
    if test -f $F -a -r $F  #Se file leggibile
    then
        L=`wc -c < $F`  #Memorizzo in L la lunghezza dei caratteri del file
        if test `expr $L % $B` -eq 0  #Se L è multiplo di B
        then
            fileValidoTrovato=true  #Ho trovato file valido
            files="$files $F"  #appendo file all'elenco dei file validi
            continue  #passo a contenuto successivo
        fi
    fi

    if test -d $F -a -x $F #Se directory traversabile
        then
            subDirTrovata=true  #ho trovato subdir
            continue    #passo a contenuto successivo
    fi
done



#La chiamata al file C e la stampa avviene solo con dir valida, ovvero entrambe specifiche a true
if test $fileValidoTrovato = true -a $subDirTrovata = true
then
    echo "Ho trovato la directory valida `pwd`"
    for F in $files
    do
        L=`wc -c < $F`
        #Creo file richiesto
        > $F.Chiara

        #fingo richiamo a file C
        echo "main.c $F $L $B "
    done
    echo "\n"
fi


#CONTINUA ESPLORAZIONE RICORSIVA
for F in *
do
    if test -d $F -a -x $F  #Se subdirectory traversabile, continuo esplorazione ricorsiva
    then
        $0 `pwd`/$F $B  #richiamo file ricorsivo fornendo subdirectory come gerarchia e B
    fi
done