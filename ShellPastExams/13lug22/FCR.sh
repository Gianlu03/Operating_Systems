#!/bin/sh
#FCP.sh
#File Comandi Ricorsivo che risolve

#PARAMETRI
# $1 = gerarchia  $2 = intero L  $3 = filetemporaneo  $n = caratteri successivi

#mi posiziono nella gerarchia passata
cd $1
#In seguito memorizzo i parametri precedenti all'elenco in variabili
L=$2    
fileTemp=$3
shift 3  #Posso effettuare 3 shift per mantenere solo l'elenco dei caratteri in $*

#ora i caratteri sono in $*
valido=
#Scorro i file nella directory (voglio che siano tutti validi)
for F in *
do
    valido=true     #variabile per flaggare se un file è valido o no
    if test -f $F -a -r $F  #Controllo se file leggibile con L righe
    then
        nRighe=`wc -l < $F`
        if test $nRighe -eq $L
        then
            echo "`pwd`/$F ha le linee"
            for i in $*  #Effettuo verifica per ogni carattere
            do
                grep $i $F > /dev/null 2>&1 #mando in /dev/null per non avere stampe su linea di comando
                if test $? -ne 0  #Se il grep di un solo carattere non va a buon fine, il file non è già valido
                then
                    valido=false
                    break;  #Interrompo ciclo, un file non valido rende già la directory non valida
                fi
            done
        else
            valido=false
        fi
        
        #Se il file è valido, quindi lo appendo alla lista e all'elenco temporaneo
        if test $valido = true
        then
            echo `pwd`/$F >> $fileTemp #memorizzo nel temporaneo il nome assoluto del file
        fi
    fi          
done

#Proseguo ricorsione nelle subdirectory se presenti
for F in *
do
    if test -d $F -a -x $F #Per ogni subdirectory traversabile
    then
        $0 `pwd`/$F $L $fileTemp $*  #richiamo ricorsione con medesimi parametri, a parte gerarchia che diventa la subdirectory
    fi          
done
