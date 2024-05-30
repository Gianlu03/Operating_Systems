#!/bin/sh
#FCP.sh
#File comandi principale per simulazione del 21 aprile 2023

#Effettuo controllo numero di parametri lasco - almeno 4 parametri
if test 4 -gt $#
then
    echo "Errore: Non sono stati inseriti 4 parametri"
    echo "USAGE: $0 stringa1 stringa2 GerAss1 GerAss2 ...\n"
    exit 1
else
    echo "DEBUG: numero parametri corretto($#), proseguo esecuzione"
fi

#preparo variabili per contenere parametri separatamente
S1=  #variabile per primo parametro - stringa semplice
S2=  #variabile per secondo parametro - stringa semplice
gerarchie=  #variabile per parametri successivi - gerarchie assolute
count=1  #contatore per distinguere parametri

#Separo parametri nelle rispettive variabili
for i in $*
do
    #Primo parametro memorizzato in S1
    if test $count -eq 1
    then
        S1=$i
        count=`expr $count + 1`
        continue
    fi 
    #Secondo parametro memorizzato in S2
    if test $count -eq 2
    then
        S2=$i
        count=`expr $count + 1`
        continue
    fi 

    gerarchie="$gerarchie $i"
    count=`expr $count + 1`
done

echo "$S1  $S2   $gerarchie"

#CONTROLLI SUI SINGOLI PARAMETRI

#Controllo se S1 e S2 sono stringhe semplici
case $S1 in
*/*)
#se compare uno / non è una stringa semplice utilizzabile
    echo "Errore: il primo parametro deve essere una stringa semplice"
    echo "USAGE: $0 stringa1 stringa2 GerAss1 GerAss2 ...\n"
    exit 2
;;
*)
    echo "DEBUG: Il primo parametro è stato inserito correttamente"
;;
esac

case $S2 in
*/*)
#se compare uno / non è una stringa semplice utilizzabile
    echo "Errore: il secondo parametro deve essere una stringa semplice"
    echo "USAGE: $0 stringa1 stringa2 GerAss1 GerAss2 ...\n"
    exit 3
;;
*)
    echo "DEBUG: Il secondo parametro è stato inserito correttamente"
;;
esac

#Effettuo controllo sulle Q gerarchie, devono essere nomi assoluti
for i in $gerarchie
do
    case $i in
    /*)
    #nome assoluto(inizia per /)
    if test ! -d $i -o ! -x $i #Verifico se è una directory
    then
        echo "Errore: la gerarchia $i non è una directory traversabile"
        echo "USAGE: $0 stringa1 stringa2 GerAss1 GerAss2 ...\n"
        exit 4
    fi
        ;;
        *)
        #Qui il parametro non è nome assoluto
        echo "Errore: la gerarchia $i non è un nome assoluto"
        echo "USAGE: $0 stringa1 stringa2 GerAss1 GerAss2 ...\n"
        exit 5
        ;;
    esac
done

echo "DEBUG: tutti i parametri inseriti sono validi"

#Tutti i parametri sono validi, proseguo aggiornando il path
PATH=`pwd`:$PATH
export PATH

#Creo i file temporanei in cui memorizzare i file
> /tmp/nomiAssoluti1$$ #File con estensione S1
> /tmp/nomiAssoluti2$$ #File con estensione S2

echo "DEBUG: INIZIO RICORSIONE"

#Effettuo ciclo per eseguire ricorsione su ogni gerarchia
for G in $gerarchie
do
    echo "Ricorsione su $G:"
    FCR.sh $S1 $S2 $G /tmp/nomiAssoluti1$$ /tmp/nomiAssoluti2$$
done

#Memorizzo numero file validi nelle variabili richieste
TOT1=`wc -l < /tmp/nomiAssoluti1$$`
TOT2=`wc -l < /tmp/nomiAssoluti2$$`

#mostro numero file trovati globalmente
echo "In totale sono stati trovati:"
echo "$TOT1 file con estensione $S1"
echo "$TOT2 file con estensione $S2"

#Verifico che operazione svolgere sulla base dei totali
if test $TOT2 -gt $TOT1
then
    valido=false #variabile per controllare parametro X, voglio sia tra 1 e TOT1
    while test $valido = false #finchè a falso viene richiesto X
    do
        echo "Gianluca, inserisci un valore intero X compreso tra 1 e $TOT1"
        read X
        case $X in
        *[!0-9]*)
            #caso non numerico
            echo "Errore: X non è un valore numerico, reinserimento"
            valido=false
        ;;
        *) #caso numerico, deve essere tra 1 e TOT1
            if test $X -ge 1 -a $X -le $TOT1
            then
                valido=true
            else
                echo "Errore: X è numerico ma non si trova nel range richiesto"
                valido=false
            fi
        ;;
        esac
    done

    #Una volta qui il parametro è valido
    echo "Ecco i 2 file in posizione X-esima:"
    head -$X /tmp/nomiAssoluti1$$ | tail -1
    head -$X /tmp/nomiAssoluti2$$ | tail -1


else
    echo "Non svolgo alcuna operazione sulla base dei totali"
fi

echo "DEBUG: FINE ESECUZIONE"

#Rimuovo i file temporanei
rm /tmp/nomiAssoluti1$$
rm /tmp/nomiAssoluti2$$