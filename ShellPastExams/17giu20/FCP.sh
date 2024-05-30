#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve ESAME del 17 giugno 2020

USAGE="USAGE: ./FCR.sh GerAss1 GerAss2 ... InteroPositivoB \n"  #formato USAGE stampato in caso di errore dell'utente nei parametri

#=================CONTROLLI NUMERO PARAMETRI===================

#CONTROLLO NUMERO PARAMETRI LASCO

#Controllo numero parametri lasco: almeno 3
if test 3 -gt $#
then
    echo "Errore: Non sono stati inseriti almeno 3 parametri"
    echo $USAGE
    exit 1
else
    echo "DEBUG: numero parametri corretto($#), proseguo esecuzione"
fi

#Separo i parametri ottenuti
counter=1 #variabile per capire a quale parametro mi trovo per separare parametri in input
gerarchie=  #Variabile "lista" per contenere le gerarchie ricevute
B= #Variabile per contenere intero positivo

for i in $*
do
    if test $counter -eq $# #se mi trovo all'ultimo parametro
    then
        B=$i #memorizzo ultimo parametro
        break
    fi
    gerarchie="$gerarchie $i" #appendo gerarchia alla lista
    counter=`expr $counter + 1` #incremento contatore
done


#Effettuo controlli sui parametri forniti

#Controllo che le prime Q gerarchie siano nomi assoluti di dir traversabili
for i in $gerarchie
do
    case $i in
    /*)
        #nome assoluto(inizia per /)
        if test ! -d $i -o ! -x $i #Verifico se è una directory traversabile
        then
            echo "Errore: il parametro $i non è una directory traversabile"
            echo $USAGE
            exit 2
        fi
    ;;
    *)
        #Qui il parametro non è nome assoluto
        echo "Errore: il parametro $i non è un nome assoluto"
        echo $USAGE
        exit 3
        ;;
    esac
done

#Effettuo controllo ultimo valore intero positivo
case $B in
*[!0-9]*)   #caso non numerico
    echo "Errore: l'ultimo parametro $B non è numerico"
    echo $USAGE
    exit 4
;;
*)  #caso numerico, devo escludere il valore nullo
    if test $B -eq 0
    then
        echo "Errore: l'ultimo parametro deve essere strettamente positivo, è stato inserito 0"
        echo $USAGE
        exit 5
    fi
    #Else -> positivo, è valido

;;
esac

echo "SONO STATI INSERITI CORRETTAMENTE TUTTI I PARAMETRI, PROSEGUO:\n"

#Dato che tutti i parametri sono validi, aggiorno $PATH
#AGGIORNAMENTO PATH
PATH=`pwd`:$PATH
export PATH

#Richiamo componente ricorsiva con una gerarchia per volta
for G in $gerarchie
do
    echo "Ricorsione su $G:"
    #Richiamo componente ricorsiva con gerarchia e intero B
    FCR.sh $G $B    
done

echo "====TERMINE ESECUZIONE===="