#!/bin/sh
#FCP.sh
#File Comandi Ricorsivo che risolve

#PARAMETRI
# $1 = Gerarchia  $2 = stringa S  $3 = temporaneo   

#mi posiziono nella gerarchia passata
cd $1
S=$2 #memorizzo stringa in variabile più significativa

for F in *
do
    if test -f $F -a -r $F  #Se file leggibile
    then
        case $F in #verifico nome file
        *.$S) #Il file ha terminazione .S = valido
            pwd >> $3  #Memorizzo dir in temporaneo
            break  #Interrompo perchè la dir è già valida
        ;;
        *)
            continue #File non valido, passo al prossimo
        ;;
        esac
    fi
done

#Continuo esplorazione ricorsiva
for F in *
do
    if test -d $F -a -x $F  #Se directory traversabile, continuo esplorazione ricorsiva
    then
        $0 `pwd`/$F $S $3  #richiamo file ricorsivo fornendo subdirectory come gerarchia
    fi
done