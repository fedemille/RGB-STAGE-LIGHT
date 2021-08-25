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
  
