#!/bin/sh
#FCR.sh
#File comandi ricorsivo per simulazione del 21 aprile 2023

#PARAMETRI:
# $1 = S1  $2 = S2  $3 = Gerarchia  $4 = file temp1   $5 file temp2

#Mi posiziono nella directory fornita
cd $3

echo "Sono nel ricorsivo con `pwd` \n"

#Ora effettuo ricerca file idonei
for F in *
do
    #Verifico se file leggibile
    if test -f $F -a -r $F
    then
        case $F in
        *.$1)  #Se ha estensione S1 scrivo nel temp1
            echo "trovato il file valido `pwd`/$F"
            echo `pwd`/$F >> $4
        ;;
        *.$2)  #Se ha estensione S2 scrivo nel temp2
            echo "trovato il file valido `pwd`/$F"
            echo `pwd`/$F >> $5
        ;;
        *)
            #Se non ha estensione giusta non svolgo nulla
        ;;
        esac
    fi
done

#SPECIFICA: LE SOTTODIRECTORY SONO CERCATE IN UN ULTERIORE
#           CICLO PER AVERE UNA STAMPA PER LIVELLI DELLA GERARCHIA

#Cerco sottodirectory per proseguire esplorazione
for F in *
do
    #Verifico se file leggibile
    if test -d $F -a -x $F
    then
        $0 $1 $2 `pwd`/$F $4 $5 #richiamo ricorsivamente
    fi
done
