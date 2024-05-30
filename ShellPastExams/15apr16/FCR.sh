#!/bin/sh
#FCP.sh
#File Comandi Ricorsivo che risolve esame del 15 aprile 2016

#PARAMETRI
# $1 = gerarchia   $2 = X  $3 = file temporaneo   

#mi posiziono nella gerarchia passata
cd $1
X=$2 #Memorizzo in X il parametro intero per chiarezza di nomi

dirValida=true  #Variabile "booleana" per indicare directory valida o no
for F in *
do
    if test -f $F -a -r $F  #Se file e leggibile
    then
        nRighe=`wc -l < $F` #Recupero numero righe file
        if test $nRighe -gt $X  #Se le righe sono > X
        then
            continue  #Per ora la dir è valida, proseguo a controllare contenuto restante
        else
            dirValida=false
            break  #Non serve controllare successivi, non è valida
        fi
    else
        dirValida=false
        break
    fi
done

if test $dirValida = true  #Se directory valida, la scrivo nel temporaneo
then
    pwd >> $3 #Se non è valida, provo a continuare esplorazione in caso di subdir presenti
fi    

for F in *
do
    if test -d $F -a -x $F  #Se directory traversabile, continuo esplorazione ricorsiva
    then
        $0 `pwd`/$F $2 $3  #richiamo file ricorsivo fornendo subdirectory come gerarchia
    fi
done

