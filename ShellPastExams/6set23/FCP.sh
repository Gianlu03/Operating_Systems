#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve esame shell del 6 settembre 2023

USAGE="USAGE: FCP,sh GerAssoluta stringaSem1 stringaSem2 ... \n"  #formato USAGE stampato in caso di errore dell'utente nei parametri

#=================CONTROLLI NUMERO PARAMETRI===================

#Controllo numero parametri lasco: almeno 3
if test 3 -gt $#
then
    echo "Errore: Non sono stati inseriti almeno 3 parametri"
    echo $USAGE
    exit 1
else
    echo "DEBUG: numero parametri corretto($#), proseguo esecuzione"
fi


#Controllo che la prima stringa sia un nome assoluto
case $1 in
/*)
    #nome assoluto(inizia per /)
    if test ! -d $1 -o ! -x $1 #Verifico se è una directory traversabile
    then
        echo "Errore: il parametro $1 non è una directory traversabile"
        echo $USAGE
        exit 2
    fi
;;
*)
    #Qui il parametro non è nome assoluto
    echo "Errore: il parametro $1 non è un nome assoluto"
    echo $USAGE
    exit 3
    ;;
esac

echo "DEBUG: PRIMO PARAMETRO VALIDO"

G=$1 #memorizzo gerarchia a parte
shift  #in questo modo in $* rimangono solo le stringe

echo $G
echo $M
echo "PARAMETRI $*"


#Controllo che i successivi parametri siano tutti stringhe semplici
for i in $*
do
    case $i in
    */*)  #nome assoluto/relativo non semplice
        echo "Errore: la stringa $i non è un nome relativo semplice"
        exit 4
    ;;
    *)	
    ;;
    esac
done

M=`expr $# / 2` #memorizzo il numero corrispondente a metà delle stringhe

echo "DEBUG: TUTTI I PARAMETRI VALIDI"

#Tutti i parametri sono validi, proseguo aggiornando il path
PATH=`pwd`:$PATH
export PATH

#Parte ricorsiva
FCR.sh $G $M $*  #passo la gerarchia in cui ricercare, M e le stringhe da cercare come nomi di file

echo "FINE ESECUZIONE"

