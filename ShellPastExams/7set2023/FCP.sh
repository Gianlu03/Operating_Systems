#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve esame del 7 settembre 2022

USAGE="USAGE: $0 InteroPositDispari GerAss1 GerAss2 ... \n"  #formato USAGE stampato in caso di errore dell'utente nei parametri

#Controllo numero parametri lasco: almeno 3
if test 3 -gt $#
then
    echo "Errore: Non sono stati inseriti almeno 3 parametri"
    echo $USAGE
    exit 1
else
    echo "DEBUG: numero parametri corretto($#), proseguo esecuzione"
fi

#Controllo che il primo parametro sia intero positivo
case $1 in
*[!0-9]*)   #Caso non numerico
    echo "Errore: il primo parametro non è numerico"
    echo $USAGE
    exit 2
;;
*)  #caso numerico >= 0, devo escludere lo 0
    if test $1 -eq 0
    then
        echo "Errore: il primo parametro deve essere strettamente positivo, inserito 0"
        exit 3
    fi
    #Controllo se dispari
    if test `expr $1 % 2` -eq 0
    then
        echo "Errore: l'intero inserito non è dispari"
        exit 4
    else
        echo "DEBUG: Primo parametro valido\n"
    fi  
;;
esac

X=$1   #memorizzo primo parametro in variabile X come richiesto
shift   #Rimuovo primo parametro dalla lista

gerarchie=$*  #Memorizzo lista rimanente in variabile più significativa

for i in $gerarchie
do
    case $i in
    /*)
        #nome assoluto(inizia per /)
        if test ! -d $i -o ! -x $i #Verifico se è una directory traversabile
        then
            echo "Errore: la gerarchia $i non è una directory traversabile"
            echo $USAGE
            exit 5
        fi
    ;;
    *)
        #Qui il parametro non è nome assoluto
        echo "Errore: la gerarchia $i non è un nome assoluto"
        echo $USAGE
        exit 6
        ;;
    esac
done


echo "DEBUG: TUTTI I PARAMETRI VALIDI\n"

#tutti i parametri validi, proseguo con aggiornamento path
PATH=`pwd`:$PATH
export PATH

#Creo file temporaneo per memorizzare file validi
> /tmp/nomiAssoluti$$

for G in $gerarchie
do
    echo "Ricorsione su $G"
    FCR.sh $G $X /tmp/nomiAssoluti$$
done

#Visualizzo i file in posizione dispari nell'elenco
count=1
for i in `cat /tmp/nomiAssoluti$$`
do
    if test `expr $count % 2` -eq 1 #se file in posizione dispari lo stampo
    then
        echo "File $count: $i"
    fi
    count=`expr $count + 1`
done


rm /tmp/nomiAssoluti$$
echo "FINE ESECUZIONE"