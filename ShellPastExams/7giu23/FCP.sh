#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve la parte in shell del 7 giugno 2023

#Effettuo controllo su numero parametri lasco: almeno 3 parametri
if test 3 -gt $#
then
    echo "Errore: Non sono stati inseriti 3 parametri"
    echo "USAGE: $0 InteroPositivoX GerAss1 GerAss2 ...\n"
    exit 1
else
    echo "DEBUG: numero parametri corretto($#), proseguo esecuzione"
fi

#Memorizzo in X primo parametro intero per poi eseguire shift
X=$1
shift #In questo modo rimangono solo le gerarchie, è più facile memorizzarle
gerarchie=  #preparo variabile per contenere gerarchie

for i in $*
do
    gerarchie="$gerarchie $i"  #memorizzo in una "lista" le gerarchie
done

#CONTROLLI SUI PARAMETRI
#X, ovvero il primo parametro deve essere intero positivo
case $X in
*[!0-9]*)   #caso non numerico
    echo "Errore: il primo parametro inserito non è un valore numerico"
    echo "USAGE: $0 InteroPositivoX GerAss1 GerAss2 ...\n"
    exit 2
;;
*)  #caso numerico, devo escludere il valore nullo
    if test $X -eq 0
    then
        echo "Errore: il primo parametro deve essere strettamente positivo, è stato inserito 0"
        echo "USAGE: $0 InteroPositivoX GerAss1 GerAss2 ...\n"
        exit 3
    else #caso intero positivo valido
        echo "DEBUG: è stato inserito correttamente il primo parametro, proseguo esecuzione\n"
    fi
;;
esac

#Controllo che le Q gerarchie siano nomi assoluti
for i in $gerarchie
do
    case $i in
    /*)
    #nome assoluto(inizia per /)
    if test ! -d $i -o ! -x $i #Verifico se è una directory traversabile
    then
        echo "Errore: la gerarchia $i non è una directory traversabile"
        echo "USAGE: $0 InteroPositivoX GerAss1 GerAss2 ...\n"
        exit 4
    fi
        ;;
        *)
        #Qui il parametro non è nome assoluto
        echo "Errore: la gerarchia $i non è un nome assoluto"
        echo "USAGE: $0 InteroPositivoX GerAss1 GerAss2 ...\n"
        exit 5
        ;;
    esac
done

#TUTTI I PARAMETRI VALIDI
echo "DEBUG: tutti i parametri validi"

#Visto che tutti i parametri sono inseriti correttamente, procedo aggiornando PATH
PATH=`pwd`:$PATH
export PATH

#Preparo file temporaneo su cui scrivere durante la ricorsione
> /tmp/tmp$$

#Richiamo componente ricorsiva con una gerarchia per volta
for G in $gerarchie
do
    echo "Ricorsione su $G:"
    FCR.sh $G $X 1 /tmp/tmp$$   #oltre gerarchia e intero, passo file temporaneo e 0, valore che indica il livello della gerarchia
done

echo "Tutte le directory trovate a livello $X sono: "
cat /tmp/tmp$$

echo "DEBUG: FINE ESECUZIONE"

#rimuovo file temporaneo creato
rm /tmp/tmp$$