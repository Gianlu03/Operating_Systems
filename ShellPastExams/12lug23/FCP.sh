#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve l'esame shell del 12 luglio 2023

#formato USAGE stampato in caso di errore dell'utente nei parametri
USAGE="USAGE: InteroPosMin200 GerAss1 GerAss2 ... \n"  

#Controllo lasco sul numero di parametri: almeno 2
if test 2 -gt $#
then
    echo "Errore: Non sono stati inseriti almeno 2 parametri"
    echo $USAGE
    exit 1
else
    echo "DEBUG: numero parametri corretto($#), proseguo esecuzione\n"
fi

#CONTROLLI SUI PARAMETRI

#Controllo che il primo parametro sia un intero positivo minore di 200
case $1 in
*[!0-9]*)   #caso non numerico
    echo "Errore: Il primo parametro non è un parametro numerico"
    echo $USAGE
    exit 2
;;
*)  #caso numerico, devo escludere il valore nullo
    if test $1 -eq 0
    then
        echo "Errore: il primo parametro deve essere strettamente positivo, è stato inserito 0"
        echo $USAGE
        exit 3
    else #caso intero positivo valido
        echo "DEBUG: è stato inserito correttamente il primo parametro, proseguo esecuzione\n"
    fi
;;
esac

X=$1  #Assegno il primo parametro(valido) alla variabile X come richiesto da testo

#Effettuo shift per memorizzare più facilmente gli altri parametri
shift
gerarchie=$*  #memorizzo tutte le gerarchie a parte con un nome più significativo

#Controllo che le Q gerarchie siano nomi assoluti
for i in $gerarchie
do
    case $i in
    /*)
        #nome assoluto(inizia per /)
        if test ! -d $i -o ! -x $i #Verifico se è una directory traversabile
        then
            echo "Errore: la gerarchia $i non è una directory traversabile"
            echo $USAGE
            exit 4
        fi
    ;;
    *)
        #Qui il parametro non è nome assoluto
        echo "Errore: la gerarchia $i non è un nome assoluto"
        echo $USAGE
        exit 5
        ;;
    esac
done

echo "DEBUG: tutte le gerarchie sono valide, quindi lo sono tutti i parametri\n"

#Tutti i parametri validi, aggiorno PATH
PATH=`pwd`:$PATH
export PATH

#Preparo file temporaneo su cui scrivere
> /tmp/nomiAssoluti$$

#Richiamo componente ricorsiva con una gerarchia per volta

for G in $gerarchie
do
    echo "Ricorsione su $G:"
    FCR.sh $G $X /tmp/nomiAssoluti$$   #oltre gerarchia e intero, passo file temporaneo
done

nFile=`wc -l < /tmp/nomiAssoluti$$` #numeroFileTrovati
L=0  #Variabile per lunghezza massima trovata

#Ciclo per vedere se ho una nuova lunghezza massima data da un file di questa gerarchia
for i in `cat /tmp/nomiAssoluti$$`
do
    nRighe=`wc -l < $i` #prendo righe file
    if test $nRighe -gt $L  #Se il file ha n righe > L, aggiorno il massimo L
    then
        L=$nRighe
    fi
done

echo "Il numero di file trovati validi è: $nFile"
echo "La lunghezza massima L tra di essi è: $L"

#Controllo se sono trovati almeno 2 file
if test $nFile -ge 2
then
    echo "Sono stati trovati più di 2 file:"
    echo "main.c $L `cat /tmp/nomiAssoluti$$`"
else
    echo "Trovati meno di 2 file, non svolgo nulla"
fi