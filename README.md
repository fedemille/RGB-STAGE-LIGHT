# RGB-STAGE-LIGHT
Luce per uso semi-professionale DIY in ambito musicale


Essendo questa una luce da essere utilizzata quasi in luoghi nei quali è presente musica ad alti volumi, è stata progettata per quello e può dare il meglio di sè.
Consente infatti un'auto-regolazione della sensibilità del microfono adattandolo nel modo perfetto al volume della musica, senza però avere il problema della diminuzione del volume dovuta ad una parte "leggera" delle canzoni.
Può essere usata con modalità a rotazione automatiche oppure in un modo più professionale attraverso la connessione ad un DMX controller.

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
    - 16pF * 2
    - 47uF * 1
    - 10uF * 1
    - ohm 2w * 12
    - ohm 1w * 24
    - Connettori DMX maschio e femmina
    - Connettore 12v
    - Terminatori
    - LED RGB 3w 6 pin
    - Lenti 8 gradi
    - Supporti lenti
    - Microfono
    - Convertitore RS485 to TTL
    - Striscia led rossa

  ### Strutturali
  - Lastra di alluminio per il sostegno dei led, lenti, circuiti e dissipatori
  - Barra di ferro forata per la struttura

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
    1 - 17 - 33 - 49 - ecc.
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
