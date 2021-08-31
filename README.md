# RGB-STAGE-LIGHT
Luce per uso semi-professionale per eventi

![IMG_20210901_003731_718-01](https://user-images.githubusercontent.com/67070203/131586473-03bf5332-57eb-43bf-a042-0a761b9c905b.jpeg)



Luce RGB con led controllabili singolarmente ad alta potenza, con automazioni, effetti e controllo da un mixer esterno.
Progettata per essere immersa nella musica ad alto volume, include diverse funzioni come l'auto-regolazione della sensibilità del microfono, adattandolo nel modo perfetto al volume della musica, senza però avere il problema dell'eccessiva correzione del volume dovuta a momenti calmi improvvisi delle canzoni ma al tempo stesso reagendo a modifiche del volume generale.
Include inoltre la possibilità del controllo da parte di un mixer DMX professionale, così da poter adattarsi a tutte le esigenze.


Nello specifico permette di avere diverse funzioni:
  - led controllati singolarmente
  - diverse modalità con diversi effetti
  - rotazione degli effetti automatica
  - controllo con DMX
  - modalità stand-alone
  - settings per il settaggio dell'inditrizzo DMX o per la modalità stand-alone
  - microfono con rilevazione automatica dei beat



## Componenti utilizzati
  ### Elettronici e funzionali
    - Atmega328p * 1
    - SN74HC595N * 6
    - ULN2003A * 6
    - L7805 * 1
    - IRF540N * 1
    - 1N4007 Diode * 2
    - Oscillatore al quarzo 16MHz * 1
    - Switch * 1
    - 22pF * 2
    - 47uF * 1
    - 10uF * 1
    - 15 ohm 2w * 12
    - 6.8 ohm 1w * 24
    - Connettori DMX maschio e femmina * 1
    - Connettore 12v * 1
    - Terminatori 2p * 5
    - LED RGB 3w 6 pin * 36
    - Lenti 8 gradi * 36
    - Supporti lenti * 36
    - Microfono * 1
    - Convertitore RS485 to TTL * 1
    - Striscia led rossa 1.5m * 2

  ### Strutturali
  - Lastra di alluminio per il sostegno dei led, lenti, circuiti e dissipatori
  - Barra di ferro forata per la struttura
  - Dissipatori



## Librerie

  - [EEPROM](https://www.arduino.cc/en/Reference/EEPROM)
  - [Conceptinics](https://github.com/alfo/arduino-libraries/tree/master/Conceptinetics)
  
  ### Usate nel codice
  
  - [ApproxFFT](https://create.arduino.cc/projecthub/abhilashpatel121/approxfft-fastest-fft-function-for-arduino-fd4917)



## Canali DMX
  Le diverse funzioni attribuite ai vari canali DMX sono destritte qui di seguito numerate per canali. 
  In totale sono utilizzati 11 canali.
  Da moltiplicare per il canale base scelto. Ad esempio il colore blu si trova sul canale 3 e il canale base scelto è il 49, quindi per modificare il colore blu bisognerà agire sul canale 52.
  
  1. Rosso
  2. Verde
  3. Blu
  4. Striscia led rossa
  5. Intervallo tra gli effetti
  6. Modalità
  7. Automode (rotazione tra le modalità auto)
  8. Beat
  9. Background Rosso
  10. Background Verde
  11. Background Blu



## Menu
  Questa luce presenta due menu diversi. Il primo identificato dal colore verde e il secondo dal colore blu.
  Per entrare nel menu basta tener premuto il pulsante per 2 secondi. A questo punto rilasciandolo sarete entrati nel primo menu
  Per passare al secondo menu e poi per uscire bisogna ancora tenere premuto il pulsante.
  Per modificare una voce basta semplicemente premere una volta il pulsante.
  
  #### Canale DMX
  Questa è la scelta del canale base DMX dal quale iniziare a leggere i canali. Ogni led corrisponde ad un multiplo di 16 +1 (a partire dal canale 1).
  *1 - 17 - 33 - 49 - ecc.*
  Per tornare al primo valore, scorrere fino all'ultimo e premere una volta ancora

  #### Automode
  L'automode viene abilitata solo quando ci sarà un unico led acceso. 
  Tutti i led accesi significa che è attivo solo il modo DMX, quindi senza controllo automatico.
  Durante l'automode la connessione DMX sarà disabilitata e saranno presi i beat dal microfono.
  
  
  
## Automode
  In ognuna delle automode, gli effetti varieranno secondo un range rappresentato qui di seguito.
  1. Range dalla modalità 1 alla 11
  2. Modalità 3 e 4
  3. Modalità 8 e 9
  4. Modalità 5 e 6
  5. Modalità 11 e 12
  6. Range dalla modalità 1 alla 11 con tempo di intervallo fissato ad 1 secondo
  
  *Se le automode vengono impostate tramite comando manuale (il pulsante), esse disabiliteranno il DMX e si baseranno sul microfono e sulle frequenze rilevate per determinare i beat della musica. Inoltre verrà abilitato il cambio automatico dei colori in modo randomico.*
  
  
  
## Modalità
  Le modalità (o effetti) sono predefiniti e può essere variato solo il parametro dell'intervallo tra un effetto e il seguente. Il tempo di durata dell'effetto invece non è modificabile direttamente, ma solo agendo sulle direttive nel codice. Questa scelta è stata fatta per non complicare ulteriormente i menu, inoltre sono tutti valori calcolati per essere funzionali con musica intorno o sotto ai 140 bpm (Techno, house, reggaeton, ecc).
  0. Nessuna modalità. Tutto controllabile manualmente
  1. Stroboscopica
  2. Alternamento tra 6 led inferiori e i 6 led superiori
  3. Rincorsa dal basso
  4. Rincorsa dall'alto
  5. Rincorsa dal centro (verso l'esterno)
  6. Rincorsa dall'esterno (verso il centro)
  7. Rincorsa dal centro verso l'esterno e viceversa con un tempo più breve delle singole, e con un flashing dei led a lato alla fine
  8. Riempimento dall'alto
  9. Riempimento dal basso
  10. *VU Meter pieno, con i led accesi che partono dal basso. Se il colore viene impostato a R0 G0 B0 sarà impostato un colore di default (8 led verdi, 2 gialli e 2 rossi con flashing)
  11. *VU Meter a punti, con soli due led accesi. Se il colore viene impostato a R0 G0 B0 sarà impostato un colore di default (8 led verdi, 2 gialli e 2 rossi con flashing)
  12. *Psichedelica. Tutto rosso per il beat, due led viola semicentrali per i medi e due led azzurri esterni per gli alti
  13. Cambio graduale dei colori

  *\*basati sul microfono e sull'analisi spettrometrica delle frequenze*






## Schema
  Schema del circuito centrale, nel quale si trova l'atmega328p, con le diverse connessioni al microfono, al DMX, all'alimentazione e alle strisce led.
![schemaRGBSTAGELIGHT](https://user-images.githubusercontent.com/67070203/130881474-e93b4121-a1b9-44df-92a7-d519e544fb61.png)




## Circuiti
Schema dei circuiti per ogni 2 gruppi di led
![schemaRGBSTAGELIGHTcircuit](https://user-images.githubusercontent.com/67070203/130877291-44b75738-3f1a-4305-8f49-6e4e1e7075d4.png)



## LEDs
Schema di ogni gruppo di led RGB
La resistenza in serie ai led rossi dovrà essere di 6.8 ohm 2w, in quanto il led di colore rosso richiede una tensione minore rispetto agli altri due.
![schemaRGBSTAGELIGHTleds](https://user-images.githubusercontent.com/67070203/130881465-104e17af-7000-4a23-b5bb-beba46111adc.png)



## Altri dati
  Consumo max: 160w
  
  ### Problemi noti
  - [ ] Dissipatori sottodimensionati, il calore dissipato dai led è troppo alto e farebbe raggiungere una temperatura di oltre 80°C in meno di 10 minuti alla massima luminosità costante
  - [ ] In alcuni casi particolarmente sfavorevoli gli effetti dei led manifestano un piccolo ritardo di alcuni millisecondi
  - [ ] L'algorimo FFT, per quanto sia ottimizzato e approssimato, occupa il processore per diversi millisecondi. Si potrebbe risolvere separando i calcoli.
  - [ ] Dissipazione di calore e potenza dovuto all'uso delle resistenze per i led. Si potrebbe creare un alimentatore ad hoc alla giusta tensione per l'alimentazione dei led.
  



# Foto e Video




https://user-images.githubusercontent.com/67070203/131044544-d58b89ad-13d6-408c-add6-01e6e3a2534e.mp4

![20210831231323_IMG_9093](https://user-images.githubusercontent.com/67070203/131586600-0072d851-771d-401f-bfb6-b2cd56dc3c87.JPG)
![20210831231324_IMG_9095](https://user-images.githubusercontent.com/67070203/131586604-34142f85-c1a0-4cd8-9efb-00af6e3c1800.JPG)







