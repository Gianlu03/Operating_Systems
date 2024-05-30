#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve esame bash del 13 luglio 2022

#Definisco stringa USAGE da stampare in caso di errori da parte dell'utente nell'inserimento parametri
USAGE="USAGE: ./FCP.sh GerAssoluta InteroPositivo char1 char2 ...  \n"  #formato USAGE stampato in caso di errore dell'utente nei parametri

#Controllo numero parametri lasco: almeno 3
if test 4 -gt $#
then
    echo "Errore: Non sono stati inseriti almeno 4 parametri"
    echo $USAGE
    exit 1
else
    echo "DEBUG: numero parametri corretto($#), proseguo esecuzione"
fi

#Controllo che il primo parametro sia una directory eseguibile in nome assoluto
case $1 in
/*)
    #nome assoluto(inizia per /)
    if test ! -d $1 -o ! -x $1 #Verifico se è una directory traversabile
    then
        echo "Errore: la gerarchia $1 non è una directory traversabile"
        echo $USAGE
        exit 2
    fi
;;
*)
    #Qui il parametro non è nome assoluto
    echo "Errore: la gerarchia $i non è un nome assoluto"
    echo $USAGE
    exit 3
    ;;
esac

G=$1  #Memorizzo primo parametro in variabile a parte  

#Controllo che il secondo parametro(ora il primo) sia un intero positivo

case $2 in
*[!0-9]*)   #caso non numerico
    echo "Errore: Il secondo parametro $2 non è numerico"
    echo $USAGE
    exit 4
;;
*)  #caso numerico, devo escludere il valore nullo
    if test $2 -eq 0
    then
        echo "Errore: il secondo parametro $2 deve essere strettamente positivo"
        echo $USAGE
        exit 5
    else #caso intero positivo valido
        echo "DEBUG: è stato inserito correttamente il secondo parametro, proseguo esecuzione\n"
    fi
;;
esac

L=$2 #Memorizzo il secondo parametro intero positivo in una variabile a parte

#Effettuo due shift così da avere in $* solo l'elenco di Q caratteri
shift 2

#Effettuo controllo che tutti i successivi parametri siano singoli caratteri
for i in $*
do
    case $i in 
    ?)  #caso singolo carattere - valido
    ;;
    *)  
        echo "Errore: il parametro $i non è un singolo carattere"
        echo $USAGE
        exit 6
    ;;
    esac
done

#ora i caratteri si trovano in $*

echo "DEBUG: TUTTI I PARAMETRI INSERITI CORRETTAMENTE\n"

#Tutti i parametri inseriti correttamente, proseguo aggiornando il path
PATH=`pwd`:$PATH
export PATH

#preparo il file temporaneo su cui memorizzare i dati
> /tmp/nomiAssoluti$$

#Effettuo ricorsione
FCR.sh $G $L /tmp/nomiAssoluti$$ $* #Richiamo ricorsivo con gerarchia G, intero L, file temporaneo ed elenco caratteri

#Stampo su standard output le directory valide individuate
echo "SONO STATI INDIVIDUATI `wc -l < /tmp/nomiAssoluti$$` FILE, ecco quali:"
cat /tmp/nomiAssoluti$$  

for i in `cat /tmp/nomiAssoluti$$`
do
    echo "main.c $i $L $*"  #richiamo C con ogni file, intero L e l'elenco dei caratteri
done

rm /tmp/nomiAssoluti$$  #Rimuovo file temporaneo

echo "FINE ESECUZIONE"