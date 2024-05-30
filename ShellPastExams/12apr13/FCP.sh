#!/bin/sh
#FCP.sh
#File Comandi Principale che risolve esame del 12 aprile 2013 parte shell 

USAGE="USAGE: ./FCR.sh GerAss StringaS InteroPosN\n"  #formato USAGE stampato in caso di errore dell'utente nei parametri

#=================CONTROLLI NUMERO PARAMETRI===================

#CONTROLLO NUMERO PARAMETRI stretto

#Controllo numero parametri stretto: esattamente 3
if test 3 -ne $#
then
    echo "Errore: Non sono stati inseriti 3 parametri"
    echo $USAGE
    exit 1
else
    echo "DEBUG: numero parametri corretto($#), proseguo esecuzione"
fi

#Memorizzo i parametri ricevuti nelle variabili con nomi richiesti
G=$1  #Gerarchia G
S=$2  #Stringa S
N=$3  #Intero positivo N


#Effettuo controlli sui tipi di parametri

#Controllo che il primo parametro (G) sia un nome assoluto di dir traversabile
case $G in
/*)
    #nome assoluto(inizia per /)
    if test ! -d $G -o ! -x $G #Verifico se è una directory traversabile
    then
        echo "Errore: il primo parametro $G non è una directory traversabile"
        echo $USAGE
        exit 2
    fi
;;
*)
    #Qui il parametro non è nome assoluto
    echo "Errore: il primo parametro $G non è un nome assoluto"
    echo $USAGE
    exit 3
    ;;
esac

echo "DEBUG: Primo parametro inserito correttamente"


#Controllo che la stringa S sia una stringa semplice
case $S in
*/*)
	echo "Errore: il secondo parametro $S non è una stringa"
	exit 4
;;
*)	
	echo "DEBUG: Secondo parametro inserito correttamente"
;;
esac

#Controllo che il terzo parametro sia un intero positivo
case $N in
*[!0-9]*)   #caso non numerico
    echo "Errore: Il terzo parametro inserito non è un valore NUMERICO positivo intero"
    echo $USAGE
    exit 5
;;
*)  #caso numerico, devo escludere il valore nullo
    if test $N -eq 0
    then
        echo "Errore: il terzo parametro deve essere strettamente positivo, è stato INSERITO 0"
        echo $USAGE
        exit 6
    else #caso intero positivo valido
        echo "DEBUG: è stato inserito correttamente il terzo parametro"
    fi
;;
esac

echo "DEBUG: TUTTI I PARAMETRI INSERITI CORRETTAMENTE!\n"

#Ora che tutti i parametri sono corretti, aggiorno PATH
PATH=`pwd`:$PATH
export PATH

#preparo file temporaneo
> /tmp/nomiAssoluti$$

#effettuo esplorazione ricorsiva di G
echo "Ricorsione su $G:\n"
FCR.sh $G $S /tmp/nomiAssoluti$$

#Mostro quanti direttory sono stati trovati
nDirettori=`wc -l < /tmp/nomiAssoluti$$`
echo "Sono stati trovati $nDirettori direttori validi, ecco quali sono:"
cat /tmp/nomiAssoluti$$

if test $nDirettori -gt $N
then
    echo "\nNumero direttori > N=$N"
    valido=false #variabile per controllare parametro X, voglio sia numerico e nel range dato 1-N
    while test $valido = false #finchè a falso viene richiesto il parametro
    do
        echo "Gianluca, inserisci un valore intero X compreso tra 1 e $N:"
        read X

        case $X in
        *[!0-9]*)   #caso non numerico
            echo "Errore: X non è un valore numerico, reinserimento"
            valido=false
        ;;
        *)  #caso numerico, deve essere tra 1 e N
            if test $X -ge 1 -a $X -le $N
            then 
                valido=true
            else
                echo "Errore: X è numerico ma non si trova nel range corretto, reinserire"
                valido=false
            fi
        ;;
        esac
    done
    #intero valido

    #Seleziono la directory indicata dall'utente
    dirScelta=`head -$X /tmp/nomiAssoluti$$ | tail -1`
    echo "Scelta directory $dirScelta"
    #mi posiziono in tale directory
    cd $dirScelta

    #scorro e visualizzo file validi
    for F in *
    do
        if test -f $F -a -r $F  #Se file leggibile
        then
            case $F in #verifico nome file
            *.$S) #Il file ha terminazione .S = valido
                echo "File valido `pwd`/$F, prima riga:"
                head -1 $F
            ;;
            *)
                continue #File non valido, passo al prossimo
            ;;
            esac
        fi
    done

else
    echo "\nNumero direttori <= N=$N, non faccio nulla"
fi

