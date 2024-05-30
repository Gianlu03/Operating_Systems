#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve 16 aprile 2015

USAGE="USAGE: ./FCR.sh GerAss1 GerAss2 ... InteroPositivoX \n"  #formato USAGE stampato in caso di errore dell'utente nei parametri

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

count=1 #Contatore per individuare ultimo parametro
X= #Variabile in cui contenere ultimo parametro intero positivo
gerarchie= #variabile "lista" per contenere le gerarchie

for i in $*
do
    if test $count -eq $#  #Se ultimo parametro
    then
        X=$i  #Memorizzo a parte ultimo parametro
        count=`expr $count + 1`
        continue
    fi
    gerarchie="$gerarchie $i"  #altri parametri memorizzati in gerarchie
    count=`expr $count + 1`
done


#ORA EFFETTUO CONTROLLI SUI TIPI DI PARAMETRI
#Controllo sulle N gerarchie: devono essere nomi assoluti

for i in $gerarchie
do
    case $i in
    /*)
        #nome assoluto(inizia per /)
        if test ! -d $i -o ! -x $i #Verifico se è una directory traversabile
        then
            echo "Errore: la gerarchia $i non è una directory traversabile"
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
done

echo "Tutte le gerarchie inserite correttamente"

#Effettuo controllo ultimo parametro intero positivo X
case $X in
*[!0-9]*)   #caso non numerico
    echo "Errore: L'ultimo parametro deve essere un valore numero intero positivo"
    echo $USAGE
    exit 2
;;
*)  #caso numerico, devo escludere il valore nullo
    if test $X -eq 0
    then
        echo "Errore: l'ultimo parametro deve essere strettamente positivo, è stato inserito 0"
        echo $USAGE
        exit 3
    else #caso intero positivo valido
        echo "DEBUG: è stato inserito correttamente l'ultimo parametro, proseguo esecuzione\n"
    fi
;;
esac

echo "X=$X  ger = $gerarchie"

#ARRIVATO QUI, TUTTI I PARAMETRI SONO INSERITI CORRETTAMENTE

echo "DEBUG: TUTTI I PARAMETRI CORRETTI\n"

#Aggiorno $PATH e lo esporto, così da poter richiamare FCR.sh senza ./
PATH=`pwd`:$PATH
export PATH

#Preparo file temporaneo su cui memorizzare directory valide
> /tmp/nomiAssoluti$$

#Effettuo ricorsione su tutte le gerarchie fornite
for G in $gerarchie
do
    echo "Ricorsione su $G:"
    FCR.sh $G $X /tmp/nomiAssoluti$$  #richiamo ricorsione con gerarchia, intero X e file temporaneo su cui memorizzare
done

dirAttuale=`pwd` #memorizzo la directory di partenza prima di spostari in quelle valide trovate
#A questo punto, per ogni dir valida trovata, mi ci posiziono e stampo file con riga richiesta

echo "Sono state trovate complessivamente `wc -l < /tmp/nomiAssoluti$$` directory valide\n"

for i in `cat /tmp/nomiAssoluti$$`
do
    cd $i
    echo "Nella dir $i sono stati trovati:"
    for j in * #Scorro i file della dir valida
    do
        echo "File $j, la cui X-esima riga dal fondo è:"
        tail -$X $j | head -1  #Mostro X-esima riga del file
    done
done

#Mi riposiziono alla dir iniziale
cd $dirAttuale

#Elimino file temporaneo
rm /tmp/nomiAssoluti$$

echo "FINE ESECUZIONE"
