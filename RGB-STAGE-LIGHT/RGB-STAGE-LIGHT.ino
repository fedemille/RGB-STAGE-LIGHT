
#include <Conceptinetics.h>
#include <EEPROM.h>

/*
	MAX485
	
	RO -> Pin 0
	RE -> GND
	DE -> GND
	DI -> N.C.
	
	A -> Data +
	B -> Data -
	VCC -> 3.3v / 5v
	
	DMX
		1 -> GND
		2 -> data -
		3 -> data +

*/


//#define MEGA
#define UNO



#define SLAVE_CHANNELS 14
//#define RXEN_PIN       6


#define LED_STRIP 7

#define BUTTONPIN 2

#define MICPIN A0

#define SRdata 8			// dati 74HC595N
#define SRclk 9				// clock 74HC595N
#define SRlatchSX 10		// latch 74HC595N


DMX_Slave dmx_slave ( SLAVE_CHANNELS );
uint8_t DMXaddress = 1;		// start address for dmx. Step 16


#define RANGE 40		// possibili sfumature per ogni colore

#define LEDNO 48
byte ledHP[LEDNO];		// 12 * RGBN		rgb+nothing
uint8_t generalColorHP[3] = {0, 0, 0};
uint8_t bgColorHP[3] = {0, 0, 0};







// tempi per le modality
#define RINCORSASUGIU_T 20
#define RINCORSADAVERSOCENTRO_T 40
#define RINCORSAEXTCENTROAUTO_T 35
#define RIEMPIMENTO_T 32




#define AUTOMODECOUNT 7	// quante modalità automatiche ci sono, inclusa DMX mode



/// Settings

int stroboTime = -1;			// tempo che rimane spenta in ms
const int STROBO_ON_TIME = 32;	// tempo che rimane accesa in ms (32)
int8_t modality = 0;				// modalità luci HIGH POWER
bool randomColors = false;		// se i colori devono cambiare in modo casuale
uint8_t autoMode = 0;			// Per la modalità automatica, senza il DMX. 0-> DMX mode (all on) | 1-> All modality. Aggiornare AUTOMODECOUNT
bool micEnabled = false;		// microfono abilitato




/// State variables

unsigned long stroboStartTime = 0;
bool stroboOn = false;
uint32_t ledShiftTime = 0;
bool lastPartOn = true;		// per la modality 1
int8_t lastSumLed4 = 0;		// per la modality 4/5
int8_t ledDone6 = 0;		// per la modality 6/7
uint16_t changeColor11 = 0;	// per la modality 11
uint8_t modeChangeBeatCounter = 0;		// contatore di beat per autoMode
bool lastBeatDMX = false;	// per tenere l'ultimo stato del channel beat


bool beat = false, mid = false, high = false;	// se ha trovato un suono basso, medio o alto



// Calcoli per AUTOREGOLAZIONE MICROFONO. Il microfono si autoregola in base al volume della musica.
// setta automaticamente dei parametri temporanei del valore massimo trovato in un periodo e regolare di conseguenza il massimo generale.
// La regolazione del massimo viene ponderata in base all'incremento o al decremento, differenziando le due condizioni. (si abbassa più lentamente per favorire
//		i punti bassi delle canzoni ma si alza più velocemente per permettere un giusto adattamento all'aumento del volume)
uint16_t micValue = 0;	// valore del microfono in quell'istante

int8_t ledOnVUmeter = 0;		// led accesi nel VU Meter
uint16_t actualMicValue = 0;	// var per la media nel calcolo del VU Meter

uint16_t maxValMIC = 0, avgMaxValMIC = 0;	// var del massimo nel periodo, var del massimo generale (autoadattamento). 

uint16_t countAvgMICmin = 0, minValMIC = 0;	// variabili per il calcolo del minimo (autoadattamento). minValMIC var del minimo generale
uint32_t avgMICmin = 0;

uint32_t lastCheckMIC = 0, lastMinMICCalc = 0;		// tempo per il calcolo del massimo e del minimo






bool buttonPressed = false;			// var per lo stato del pulsante
uint32_t lastButtonPressed = 0;		// salva il tempo dell'ultima volta premuto




uint32_t lastBeat = 0;		// finti beat

bool abc = true;	// led 13




uint32_t lastSampling = 0;

#define SAMPLES 32        // Must be a power of 2. Analisi FFT		32
#define SAMPLING_FREQUENCY 2500	// Analisi FFT		2500

uint16_t beatDetectorValue = 0;	// usata per il massimo nell'analisi FFT
uint16_t countMicValues = 0;	// conta i valori del microfono messi in micValues per l'analisi FFT
uint16_t micValues[SAMPLES];	// salva i sample dal microfono per l'analisi FFT
uint16_t peak = 0;		// salva il valore trovato dall'analisi FFT
uint16_t avgPeakFreq = 0,  peakFreqCounter = 0;	// media tra i picchi dell'analisi FFT















#define DMX_Address_EEPROM 0	// byte da leggere nella EEPROM
#define autoMode_EEPROM 1






inline void change(byte ledNo, byte r, byte g, byte b);		// ledNo - 0 all - n led
void setLedHP();
inline void setLedStrip(bool on);
inline bool getLedStrip();
inline float mapf(float x, float in_min, float in_max, float out_min, float out_max);
float Approx_FFT(int in[],int N,float Frequency);



void setup(){
	pinMode(LED_STRIP, OUTPUT);
	
	
	pinMode(SRclk, OUTPUT);
	pinMode(SRdata, OUTPUT);
	pinMode(SRlatchSX, OUTPUT);
	pinMode(BUTTONPIN, INPUT);
	
	//Serial.begin(115200);
	//Serial.println("Ciao");
	
	randomSeed(analogRead(A1));
	
	while(countAvgMICmin < 800){	// calcola minimo trovando la media dei valori, per non doverlo fare dopo
		avgMICmin += analogRead(MICPIN);		// riutilizza questa variabile, senza crearne una nuova
		countAvgMICmin++;
	}
	minValMIC = avgMICmin / countAvgMICmin;	// fa la media dei valori sommati
	countAvgMICmin = 0;
	avgMICmin = 0;
	avgMaxValMIC = minValMIC + 10;
	maxValMIC = 0;
	lastCheckMIC = millis();
	
	
	
	DMXaddress = EEPROM.read(DMX_Address_EEPROM);	// legge i valori dalla memoria
	autoMode = EEPROM.read(autoMode_EEPROM);
	
	if(DMXaddress == 0 || DMXaddress > 177) DMXaddress = 1;		// controlla se sono nel range giusto
	if(autoMode >= AUTOMODECOUNT) autoMode = 0;
	
	
	
	
	dmx_slave.onReceiveComplete(OnFrameReceiveComplete);
	dmx_slave.enable();						// abilita il DMX
	
	// indirizzo di partenza. Inizia da 1
	dmx_slave.setStartAddress(DMXaddress);
	
	
	change(0, 0, 0, 0);		// fino a RANGE. Setta tutto a 0
	
	
	if(autoMode != 0){ 
		micEnabled = true; 
		dmx_slave.disable();
	}
	else{
		micEnabled = false;
		dmx_slave.enable();
	}
	
	
	
	// gestione timer con interrupts
	
	cli();                      // disabilita gli interrupt
	
	TCCR1A = 0;                 // Reset entire TCCR1A to 0 
	TCCR1B = 0;                 // Reset entire TCCR1B to 0

	// divisione del clock
	TCCR1B |= B00000100;        // clock / 256 

	
	TIMSK1 |= B00000010;		// abilita la comparazione del registro col registro A

	// imposta il valore del registro
	OCR1A = 16;             // confronta il registro con questo valore

	TCNT1 = 0;
	sei();                     // attiva gli interrupt
  
}


















void loop(){
	
	if(digitalRead(BUTTONPIN) == LOW)
		buttonPressed = false;
	else{
		if(!buttonPressed){
			buttonPressed = true;
			lastButtonPressed = millis();
		}
		else if(millis() - lastButtonPressed > 2500){
			gestioneMenu();
		}
	}
	
	
	if(micEnabled){
		calcoliMic();
	}
	
	
	
	if(stroboTime != -1 && ((millis() - lastBeat) > ((uint16_t)stroboTime))) beat=true, lastBeat = millis();	// generazione di beat finti
	
	
	gestioneModalita();
	
	
	
	if(beat){		// c'è un beat
		if(autoMode != 0){		// gestione automode
			gestioneAutomode();
		}
		
		if(modality != 12)	// altrimenti andrebbe a togliere il beat a true che serve alla psichedelica
			beat = false;
		
		gestioneModalitaOnBeat();
		
		
		if(randomColors)	// cambia colore in modo random
			generalColorHP[0] = random(RANGE), generalColorHP[1] = random(RANGE), generalColorHP[2] = random(RANGE);
	}
}








#define CH(x) dmx_slave.getChannelValue(x)

void OnFrameReceiveComplete (unsigned short ch){
	// Called every time when it collected its data
	// and no interference from other interrupts
	
	
	uint8_t v;
	
	
	generalColorHP[0] = (((float)CH(1))/256.f)*RANGE;
	generalColorHP[1] = (((float)CH(2))/256.f)*RANGE;
	generalColorHP[2] = (((float)CH(3))/256.f)*RANGE;
	
	
	
	if((v = CH(4)) == 0) setLedStrip(false), randomColors = false;			// led strip
	else if(v < 128) setLedStrip(true), randomColors = false;
	else if(v == 255) setLedStrip(true), randomColors = true;
	else setLedStrip(false), randomColors = true;
	
	if((v = CH(5)) == 0){						// velocità dell'effetto
		stroboTime = -1;
	}
	else{
		stroboTime = (((float)v)/256.f) * 1500;	// da circa 11 a 1500ms
	}
	
	
	modality = (((float)CH(6))/256.f) * (13+1);				// modalità
	
	if(modality > 9 && modality < 13){
		micEnabled = true;
	}
	else{
		micEnabled = false;
		if((bool)(v = CH(8)) != lastBeatDMX){	// beat
			beat = true;
			lastBeatDMX = (bool)v;
		}
	}
	
	autoMode = (((float)CH(7))/256.f) * AUTOMODECOUNT;	// automode
	
	
	bgColorHP[0] = (((float)CH(9))/256.f) * RANGE;
	bgColorHP[1] = (((float)CH(10))/256.f) * RANGE;
	bgColorHP[2] = (((float)CH(11))/256.f) * RANGE;
	
}

















inline void gestioneMenu(){
	setLedStrip(false);
	change(0, 0, 0, 0);
	change(((DMXaddress/16)+1), 0, 1, 0);	// accende di verde il led nella posizione
	delay(1000);
	buttonPressed = false;
	bool esci = false, newMenu = false;
	uint8_t menu = 1;	// Due menu. Uno per settare il DMX address e l'altro per scegliere l'effetto automatico
	while(!esci){	// modalità setting DMX address
		if(digitalRead(BUTTONPIN) == HIGH){
			if(!buttonPressed){
				buttonPressed = true;
				lastButtonPressed = millis();
			}
			else if(millis() - lastButtonPressed > 2000 && !newMenu)	// tenere premuto due secondi per uscire dalle impostazioni
				if(menu == 1){ 
					menu = 2; 
					change(0, 0, 0, 0); 
					change(autoMode, 0, 0, 1); 
					newMenu = true; 
				}
				else	esci = true;
		}
		else{
			if(buttonPressed){	// se è stato rilasciato dopo essere stato premuto. Quindi è stato normalmente premuto senza essere stato tenuto
				if(menu == 1){
					change(0, 0, 0, 0);
					DMXaddress += 16;
					if(DMXaddress > 177)	// 177 sta nell'ultimo led, quindi se lo start address è maggiore deve tornare al primo
						DMXaddress = 1;
					change(((DMXaddress/16)+1), 0, 1, 0);	// accende di verde il led nella posizione
				}
				else if(menu == 2 && newMenu){	// per evitare che mi cambi subito impostazione quando poi lascio il pulsante (dopo che l'ho tenuto premuto per arrivare qui)
					newMenu = false;
				}
				else if(menu == 2){
					change(0, 0, 0, 0);
					autoMode++;
					if(autoMode >= AUTOMODECOUNT){
						autoMode = 0;
					}
					change(autoMode, 0, 0, 1);
				}
			}
			buttonPressed = false;
		}
		delay(10);
	}
	change(0, 0, 0, 0);
	stroboTime = -1;
	EEPROM.write(DMX_Address_EEPROM, DMXaddress);
	EEPROM.write(autoMode_EEPROM, autoMode);
	
	
	if(autoMode != 0){ 
	randomColors = true;
		if(autoMode == 6){
			micEnabled = false;
			dmx_slave.disable();
			stroboTime = 1000;
		}
		else{
			micEnabled = true; 
			dmx_slave.disable();
			stroboTime = -1;
		}
	}
	else{
		randomColors = false;
		micEnabled = false;
		dmx_slave.enable();
	}
	
	delay(2000);
	dmx_slave.setStartAddress(DMXaddress);
	lastButtonPressed = millis();
}








inline void calcoliMic(){
	micValue = analogRead(MICPIN);		// prende il valore attuale del microfono
	if(actualMicValue < micValue)		// salva solo il massimo trovato nel periodo
		actualMicValue = micValue;
	
	if(micValue > beatDetectorValue)
		beatDetectorValue = micValue;
	
	
	micValues[countMicValues] = micValue;
	countMicValues++;
	
	
	if(millis() > (lastCheckMIC + 7500)){	// alla fine di ogni periodo modifica il massimo generale
		
		if(avgMaxValMIC < maxValMIC){			// se è più alto del massimo generale allora si incrementa di un terzo
			maxValMIC -= ((maxValMIC - ((maxValMIC + minValMIC) / 2)) / 3) * 2;		// lo aumenta fino alla media 2/3 della media
			avgMaxValMIC += abs(((int16_t)maxValMIC - (int16_t)avgMaxValMIC)) / 3;		// e lo fa gradualmente, un terzo alla volta appunto
		}
		else if(avgMaxValMIC > (minValMIC + 13))	// se è più basso del massimo generale allora si decrementa di un sesto
			avgMaxValMIC -= (avgMaxValMIC - maxValMIC) / 6;
		maxValMIC = 0;
		lastCheckMIC = millis();
		
		if(millis() > (lastMinMICCalc + 25000))		// calcola il minimo. Lo fa ogni 25 secondi, ma lo controlla ogni 7.5, almeno fa meno controlli nel loop
			if(countAvgMICmin < 600){			// calcola minimo trovando la media dei valori, per non doverlo fare dopo
				avgMICmin += micValue;
				countAvgMICmin++;
			}
			else{
				minValMIC = (avgMICmin / countAvgMICmin);	// fa la media dei valori sommati
				countAvgMICmin = 0;
				avgMICmin = 0;
				lastMinMICCalc = millis();
			}
	}
	if(micValue > maxValMIC) maxValMIC = micValue;

	
	
	
	
	
	
	
	if(countMicValues >= SAMPLES){		// numero di samples per calcolare la trasformata di fourier
		double tempoImpiegato = (double)(micros() - lastSampling) / 1000000;
		double time = ((1.0/tempoImpiegato) * SAMPLES);
		peak = Approx_FFT(micValues, SAMPLES, time);		// circa 7ms
		
		
		avgPeakFreq += peak;
		peakFreqCounter++;
		
		
		
		if(peakFreqCounter == 2){
			avgPeakFreq /= peakFreqCounter;
			
			
			if(avgPeakFreq > 33 && avgPeakFreq < 95 && (beatDetectorValue > (minValMIC+((avgMaxValMIC - minValMIC)/10*8))))
				beat = true;
			else if(avgPeakFreq > 700 && avgPeakFreq < 1350 && (beatDetectorValue > (minValMIC+((avgMaxValMIC - minValMIC)/10*5))))
				mid = true;
			else if(avgPeakFreq > 1600 && (beatDetectorValue > (minValMIC+((avgMaxValMIC - minValMIC)/10*4))))
				high = true;
			
			
			peakFreqCounter = 0;
			avgPeakFreq = 0;
		}
		
		
		lastSampling = micros();
	
		
		
		countMicValues = 0;
		beatDetectorValue = 0;
		
	}
	
	
}








inline void gestioneModalita(){
	switch(modality){
		case 0: 
			if(millis() - ledShiftTime > 10){
				change(0, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
				ledShiftTime = millis();
			}
		break;
		case 1:		// strobo
			if(stroboOn && (millis() - stroboStartTime > STROBO_ON_TIME)){
				stroboOn = false;
				setLedStrip(false);
				change(0, 0, 0, 0);
			}
		break;
		case 2: 	// metà alternate
			if(stroboOn){
				for(int j=(lastPartOn?1:(LEDNO/4/2 + 1)); j<=(lastPartOn?(LEDNO/4/2):(LEDNO/4)); j++)
					change(j, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
				for(int j=(lastPartOn?1:(LEDNO/4/2 + 1)); j<=(lastPartOn?(LEDNO/4/2):(LEDNO/4)); j++)
					change(j, RANGE-generalColorHP[0], RANGE-generalColorHP[1], RANGE-generalColorHP[2]);
				lastPartOn = !lastPartOn;
				stroboOn = false;
			}
		break;
		case 3:		// rincorsa dal basso
			if(millis() - ledShiftTime > RINCORSASUGIU_T)
				if(ledDone6 <= LEDNO/4){
					change(ledDone6, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					if(ledDone6 != 1) change((ledDone6-1), bgColorHP[0], bgColorHP[1], bgColorHP[2]);
					ledDone6++;
					ledShiftTime = millis();
				}
				else if(ledDone6 == (LEDNO/4+1)){
					change(0, 0, 0, 0);		// change((LEDNO/4), 0, 0, 0);
					ledDone6++;
				}
		break;
		case 4:		// rincorsa dall'alto
			if(millis() - ledShiftTime > RINCORSASUGIU_T)
				if(ledDone6 > 0){
					change(ledDone6, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					if(ledDone6 != LEDNO/4) change((ledDone6+1), bgColorHP[0], bgColorHP[1], bgColorHP[2]);
					ledDone6--;
					ledShiftTime = millis();
				}
				else if(ledDone6 == 0){
					change(0, 0, 0, 0);		// change(1, 0, 0, 0);
					ledDone6--;
				}
		break;
		case 5:		// rincorsa verso l'esterno
			if(millis() - ledShiftTime > RINCORSADAVERSOCENTRO_T)
				if(lastSumLed4 < LEDNO/4/2){
					change(((LEDNO/4/2)-lastSumLed4+1), 0, 0, 0);
					change(((LEDNO/4/2+1)+lastSumLed4-1), 0, 0, 0);
					change(((LEDNO/4/2)-lastSumLed4), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					change(((LEDNO/4/2+1)+lastSumLed4), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					lastSumLed4++;
					ledShiftTime = millis();
				}
				else if(lastSumLed4 == LEDNO/4/2){
					change(((LEDNO/4/2)-(LEDNO/4/2)+1), 0, 0, 0);
					change(((LEDNO/4/2+1)+(LEDNO/4/2)-1), 0, 0, 0);
					lastSumLed4++;
				}
		break;
		case 6:		// rincorsa verso il centro
			if(millis() - ledShiftTime > RINCORSADAVERSOCENTRO_T)
				if(lastSumLed4 >= 0){	// parte da 5
					if(lastSumLed4 != (LEDNO/4/2-1)){
						change(((LEDNO/4/2)-lastSumLed4-1), 0, 0, 0);
						change(((LEDNO/4/2+1)+lastSumLed4+1), 0, 0, 0);
					}
					change(((LEDNO/4/2)-lastSumLed4), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					change(((LEDNO/4/2+1)+lastSumLed4), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					lastSumLed4--;
					ledShiftTime = millis();
				}
				else if(lastSumLed4 == -1){
					change((LEDNO/4/2), 0, 0, 0);
					change((LEDNO/4/2+1), 0, 0, 0);
					lastSumLed4--;
				}
		break;
		case 7:		// rincorsa verso l'esterno e poi verso il centro
			if(millis() - ledShiftTime > RINCORSAEXTCENTROAUTO_T)
				if(lastSumLed4 < LEDNO/4/2 && ledDone6 == 0){
					change(((LEDNO/4/2)-lastSumLed4+1), 0, 0, 0);
					change(((LEDNO/4/2+1)+lastSumLed4-1), 0, 0, 0);
					change(((LEDNO/4/2)-lastSumLed4), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					change(((LEDNO/4/2+1)+lastSumLed4), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					lastSumLed4++;
					setLedStrip(true);
					ledShiftTime = millis();
				}
				else if(lastSumLed4 == (LEDNO/4/2)){
					lastSumLed4--;
					ledDone6 = 1;
					ledShiftTime = millis();
				}
				else if(lastSumLed4 >= 0){
					if(lastSumLed4 != (LEDNO/4/2-1)){
						change(((LEDNO/4/2)-lastSumLed4-1), 0, 0, 0);
						change(((LEDNO/4/2+1)+lastSumLed4+1), 0, 0, 0);
					}
					change(((LEDNO/4/2)-lastSumLed4), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					change(((LEDNO/4/2+1)+lastSumLed4), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					lastSumLed4--;
					ledShiftTime = millis();
				}
				else if(lastSumLed4 == -1){
					change((LEDNO/4/2), 0, 0, 0);
					change((LEDNO/4/2+1), 0, 0, 0);
					lastSumLed4--;
					setLedStrip(false);
				}
		break;
		case 8:		// riempimento dall'alto
			if(millis() - ledShiftTime > RIEMPIMENTO_T){
				if(ledDone6 > 0){
					change(ledDone6, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					ledDone6--;
				}
				else if(ledDone6 == -1){		// così lo esegue solo una volta alla fine
					change(0, 0, 0, 0), ledDone6--;
					setLedStrip(true);
				}
				else if(ledDone6 == -2)
					setLedStrip(false), ledDone6--;
				ledShiftTime = millis();
			}
		break;
		case 9:		// riempimento dal basso
			if(millis() - ledShiftTime > RIEMPIMENTO_T){
				if(ledDone6 <= LEDNO/4){
					change(ledDone6, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					ledDone6++;
				}
				else if(ledDone6 == (LEDNO/4+1)){	// così lo esegue solo una volta alla fine
					change(0, 0, 0, 0), ledDone6++;
					setLedStrip(true);
				}
				else if(ledDone6 == (LEDNO/4+2))
					setLedStrip(false), ledDone6++;
				ledShiftTime = millis();
			}
		break;
		case 10:				// VU Meter pieno (coi led sempre accesi in basso)
		case 11:			// VU Meter (con solo due led che si muovono su e giù)
			if(millis() - ledShiftTime > 40){
				ledOnVUmeter = map(actualMicValue, minValMIC, avgMaxValMIC, 0, 12);
				actualMicValue = 0;
				if(ledOnVUmeter > 12) 		ledOnVUmeter = 12;
				else if(ledOnVUmeter < 0)	ledOnVUmeter = 0;
				change(0, 0, 0, 0);
				if(generalColorHP[0] == 0 && generalColorHP[1] == 0 && generalColorHP[2] == 0){
					setLedStrip(false);
					if(modality == 10)
						for(uint8_t i=1; i<=ledOnVUmeter; i++)
							if(i < 9)
								change(i, 0, RANGE, 0);
							else if(i < 11)
								change(i, RANGE, RANGE, 0);
							else{
								change(i, RANGE, 0, 0);
								setLedStrip(true);
							}
					else
						if(ledOnVUmeter < 9){
							change(ledOnVUmeter, 0, RANGE, 0);
							if(ledOnVUmeter != 1) change((ledOnVUmeter-1), 0, RANGE, 0);
						}
						else if(ledOnVUmeter < 11)
							change(ledOnVUmeter, RANGE, RANGE, 0), change((ledOnVUmeter-1), RANGE, RANGE, 0);
						else{
							change(ledOnVUmeter, RANGE, 0, 0), change((ledOnVUmeter-1), RANGE, 0, 0);
							setLedStrip(true);
						}
				}
				else
					if(modality == 10)
						for(uint8_t i=1; i<=ledOnVUmeter; i++)
							change(i, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					else{
						change(ledOnVUmeter, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
						if(ledOnVUmeter != 1) change((ledOnVUmeter-1), generalColorHP[0], generalColorHP[1], generalColorHP[2]);
					}
				ledShiftTime = millis();
			}
		break;
		case 12:		// Psichedelica (colori diversi in base alla frequenza)
			if(millis() - ledShiftTime > 50){
				change(0, 0, 0, 0);
				setLedStrip(false);
				if(beat)
					change(0, RANGE, 0, 0), setLedStrip(true), beat = false;
				if(mid)
					change(5, RANGE, 0, RANGE), change(8, RANGE, 0, RANGE), mid = false;
				if(high)
					change(12, 0, RANGE, RANGE), change(1, 0, RANGE, RANGE), high = false;
				ledShiftTime = millis();
			}
		break;
		case 13:	// cambio graduale dei colori
			if(millis() - ledShiftTime > 30){
				if(changeColor11 < (RANGE))
					change(0, RANGE, (changeColor11 - (RANGE)), 0);
				else if(changeColor11 < (RANGE * 2))
					change(0, RANGE - (changeColor11 - (RANGE * 2)), RANGE, 0);
				else if(changeColor11 < (RANGE * 3))
					change(0, 0, RANGE, (changeColor11 - (RANGE * 3)));
				else if(changeColor11 < (RANGE * 4))
					change(0, 0, RANGE - (changeColor11 - (RANGE * 4)), RANGE);
				else if(changeColor11 < (RANGE * 5))
					change(0, (changeColor11 - (RANGE * 5)), 0, RANGE);
				else if(changeColor11 < (RANGE * 6))
					change(0, RANGE, 0, RANGE - (changeColor11 - (RANGE * 6)));
				else
					changeColor11 = 0;
				changeColor11++;
				ledShiftTime = millis();
			}
		break;
	}
}



inline void gestioneModalitaOnBeat(){
	switch(modality){
		case 0:		// nessun effetto
			
		break;
		case 1:		// strobo
			change(0, generalColorHP[0], generalColorHP[1], generalColorHP[2]);
			setLedStrip(true);
			stroboOn = true;
			stroboStartTime = millis();
		break;
		case 2:
			stroboOn = true;
		break;
		case 3:		// rincorsa dal basso
			ledDone6 = 1;
			ledShiftTime = 0;
		break;
		case 4:		// rincorsa dall'alto
			ledDone6 = LEDNO/4;
			ledShiftTime = 0;
		break;
		case 5:		// rincorsa dal mezzo
			lastSumLed4 = 0;	// somma da aggiungere/togliere per accendere quelli vicini
			change(0, 0, 0, 0);
			ledShiftTime = 0;
		break;
		case 6:		// rincorsa dall'esterno
			lastSumLed4 = LEDNO/4/2-1;	// somma da aggiungere/togliere per accendere quelli vicini
			change(0, 0, 0, 0);
			ledShiftTime = 0;
		break;
		case 7:		// rincorsa verso l'esterno e poi verso il centro
			lastSumLed4 = 0;
			ledDone6 = 0;
			change(0, 0, 0, 0);
			ledShiftTime = 0;
		break;
		case 8:		// riempimento in basso
			ledDone6 = LEDNO/4;
			change(0, 0, 0, 0);
			ledShiftTime = 0;
		break;
		case 9:		// riempimento in alto
			ledDone6 = 1;
			change(0, 0, 0, 0);
			ledShiftTime = 0;
		break;
	}
}






inline void gestioneAutomode(){
	if((autoMode == 1 || autoMode == 6) && modeChangeBeatCounter >= 4){
		modality = random(1, 12);	// 12 escluso
		modeChangeBeatCounter = 0;
	}
	else if(autoMode == 2 && modeChangeBeatCounter >= 1){
		modality = random(3, 5);	// 5 escluso
		modeChangeBeatCounter = 0;
	}
	else if(autoMode == 3 && modeChangeBeatCounter >= 1){
		modality = random(8, 10);	// 10 escluso
		modeChangeBeatCounter = 0;
	}
	else if(autoMode == 4 && modeChangeBeatCounter >= 2){
		modality = random(5, 7);	// 7 escluso
		modeChangeBeatCounter = 0;
	}
	else if(autoMode == 5 && modeChangeBeatCounter >= 8){
		modality = random(11, 13);	// 13 escluso
		modeChangeBeatCounter = 0;
	}
	modeChangeBeatCounter++;
}











inline void setLedStrip(bool on){
	if(on)
		#ifdef MEGA
			PORTH |= 0b00010000;
			#endif
		#ifdef UNO
			PORTD |= 0b10000000;
			#endif
		#ifdef ALTRO
			digitalWrite(LED_STRIP, HIGH);
		#endif
	else
		#ifdef MEGA
			PORTH &= 0b11101111;
			#endif
		#ifdef UNO
			PORTD &= 0b01111111;
			#endif
		#ifdef ALTRO
			digitalWrite(LED_STRIP, LOW);
		#endif
}

inline bool getLedStrip(){
	#ifdef MEGA
		return PORTH & 0b00010000;
		#endif
	#ifdef UNO
		return PORTD & 0b10000000;
		#endif
	#ifdef ALTRO
		return false;
	#endif
}






inline void change(byte ledNo, byte r, byte g, byte b){
	if(ledNo == 0)
		for (int i = 0; i < LEDNO; i+=4)  {
			ledHP[i+1] = r;
			ledHP[i+2] = g;
			ledHP[i+3] = b;
		}
	else{
		ledNo = (ledNo%2==0?(ledNo-2):ledNo);	// dato che dovrebbe essere ledNo-1 unito al ledNo%2==0?ledNo+1:ledNo-1
		ledNo <<= 2;	// ledNo * 4
		ledHP[ledNo+1] = r;
		ledHP[ledNo+2] = g;
		ledHP[ledNo+3] = b;
	}
}





ISR(TIMER1_COMPA_vect){
	//cli();
	TCNT1  = 0;                  // Reimposto il contatore
	setLedHP();
	//sei();
}


volatile uint8_t counter = 0;


inline void setLedHP(){
	
	if(counter >= RANGE) counter = 0;
	
	
	
	
	for (int8_t i = LEDNO-1; i >= 0; i--){
		
		if(counter < ledHP[i]){		// se è passato il tempo in cui rimanere acceso
			#ifdef MEGA
				PORTH |= 0b00100000;
			#endif
			#ifdef UNO
				PORTB |= 0b00000001;
			#endif
			#ifdef ALTRO	
				digitalWrite(SRdata, HIGH);
			#endif
		}
		else{
			#ifdef MEGA
				PORTH &= 0b11011111;
			#endif
			#ifdef UNO
				PORTB &= 0b11111110;
			#endif
			#ifdef ALTRO
				digitalWrite(SRdata, LOW);
			#endif
		}
		#ifdef MEGA
			PORTH |= 0b01000000;	// clock
			PORTH &= 0b10111111;
		#endif
		#ifdef UNO
			PORTB |= 0b00000010;	// clock
			PORTB &= 0b11111101;
		#endif
		#ifdef ALTRO
			digitalWrite(SRclk, HIGH);
			digitalWrite(SRclk, LOW);
		#endif
	}
	
	
	
	
	#ifdef MEGA
		PORTB &= 0b11101111;		// latch
		PORTB |= 0b00010000;
	#endif
	#ifdef UNO
		PORTB &= 0b11111011;		// latch
		PORTB |= 0b00000100;
	#endif
	#ifdef ALTRO
		digitalWrite(SRlatchSX, HIGH);
		digitalWrite(SRlatchSX, LOW);
	#endif
	
	counter++;
	
	
}









inline float mapf(float x, float in_min, float in_max, float out_min, float out_max){
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}










































//---------------------------------lookup data------------------------------------//
byte isin_data[128]=
{0,  1,   3,   4,   5,   6,   8,   9,   10,  11,  13,  14,  15,  17,  18,  19,  20, 
22,  23,  24,  26,  27,  28,  29,  31,  32,  33,  35,  36,  37,  39,  40,  41,  42, 
44,  45,  46,  48,  49,  50,  52,  53,  54,  56,  57,  59,  60,  61,  63,  64,  65, 
67,  68,  70,  71,  72,  74,  75,  77,  78,  80,  81,  82,  84,  85,  87,  88,  90, 
91,  93,  94,  96,  97,  99,  100, 102, 104, 105, 107, 108, 110, 112, 113, 115, 117, 
118, 120, 122, 124, 125, 127, 129, 131, 133, 134, 136, 138, 140, 142, 144, 146, 148, 
150, 152, 155, 157, 159, 161, 164, 166, 169, 171, 174, 176, 179, 182, 185, 188, 191, 
195, 198, 202, 206, 210, 215, 221, 227, 236};
unsigned int Pow2[14]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096};
byte RSSdata[20]={7,6,6,5,5,5,4,4,4,4,3,3,3,3,3,3,3,2,2,2};
//---------------------------------------------------------------------------------//


//int data[256]={};




//float f=Approx_FFT(data,512,100);
//Serial.println(f);
            


//-----------------------------FFT Function----------------------------------------------//
/*
Code to perform High speed and Accurate FFT on arduino,
setup:

1. in[]     : Data array, 
2. N        : Number of sample (recommended sample size 2,4,8,16,32,64,128,256,512...)
3. Frequency: sampling frequency required as input (Hz)

It will by default return frequency with max aplitude,
if you need complex output or magnitudes uncomment required sections

If sample size is not in power of 2 it will be clipped to lower side of number. 
i.e, for 150 number of samples, code will consider first 128 sample, remaining sample  will be omitted.
For Arduino nano, FFT of more than 256 sample not possible due to mamory limitation 
Code by ABHILASH
Contact: abhilashpatel121@gmail.com
Documentation & details: https://www.instructables.com/member/abhilash_patel/instructables/
*/

float Approx_FFT(int in[],int N,float Frequency)
{ 
int a,c1,f,o,x,data_max,data_min=0;
long data_avg,data_mag,temp11;         
byte scale,check=0;

data_max=0;
data_avg=0;
data_min=0;

      for(int i=0;i<12;i++)                 //calculating the levels
         { if(Pow2[i]<=N){o=i;} }
     a=Pow2[o];  
int out_r[a];   //real part of transform
int out_im[a];  //imaginory part of transform

      for(int i=0;i<a;i++)                //getting min max and average for scalling
          { out_r[i]=0; out_r[i]=0;
            data_avg=data_avg+in[i];
            if(in[i]>data_max){data_max=in[i];}
            if(in[i]<data_min){data_min=in[i];}
          }

data_avg=data_avg>>o;
scale=0;
data_mag=data_max-data_min;
temp11=data_mag;

for(int i;i<128;i++)              //scalling data  from +512 to -512

      if(data_mag>1024)
          {while(temp11>1024)
                {temp11=temp11>>1;
                scale=scale+1;
                }   
          }
 
      if(data_mag<1024)
          {while(temp11<1024)
                {temp11=temp11<<1;
                scale=scale+1;
                }
          }


      if(data_mag>1024)
          {
              for(int i=0;i<a;i++)
                    { in[i]=in[i]-data_avg;
                      in[i]=in[i]>>scale;
                    }
                    scale=128-scale;
          }

      if(data_mag<1024)
          { scale=scale-1;
            for(int i=0;i<a;i++)
                    {
                      in[i]=in[i]-data_avg;
                      in[i]=in[i]<<scale;
                    }

                    scale=128+scale;
          }

       
x=0;  
      for(int b=0;b<o;b++)                     // bit reversal order stored in im_out array
         {
          c1=Pow2[b];
          f=Pow2[o]/(c1+c1);
                for(int j=0;j<c1;j++)
                    { 
                     x=x+1;
                     out_im[x]=out_im[j]+f;
                    }
         }

 
      for(int i=0;i<a;i++)            // update input array as per bit reverse order
         {
          out_r[i]=in[out_im[i]]; 
          out_im[i]=0;
         }


int i10,i11,n1,tr,ti;
float e;
int c,s,temp4;
    for(int i=0;i<o;i++)                                    //fft
    {
     i10=Pow2[i];              // overall values of sine/cosine  
     i11=Pow2[o]/Pow2[i+1];    // loop with similar sine cosine
     e=1024/Pow2[i+1];  //1024 is equivalent to 360 deg
     e=0-e;
     n1=0;

          for(int j=0;j<i10;j++)
          {
            c=e*j;    //c is angle as where 1024 unit is 360 deg
  while(c<0){c=c+1024;}
  while(c>1024){c=c-1024;}

          n1=j;
          
          for(int k=0;k<i11;k++)
                 {
                   temp4=i10+n1;
       if(c==0)   {tr=out_r[temp4];
                   ti=out_im[temp4];}
  else if(c==256) {tr= -out_im[temp4];
                   ti=out_r[temp4];}
  else if(c==512) {tr=-out_r[temp4];
                  ti=-out_im[temp4];}
  else if(c==768) {tr=out_im[temp4];
                   ti=-out_r[temp4];}
  else if(c==1024){tr=out_r[temp4];
                   ti=out_im[temp4];}
  else{
    tr=fast_cosine(out_r[temp4],c)-fast_sine(out_im[temp4],c);            //the fast sine/cosine function gives direct (approx) output for A*sinx
    ti=fast_sine(out_r[temp4],c)+fast_cosine(out_im[temp4],c);            
      }
          
                 out_r[n1+i10]=out_r[n1]-tr;
                 out_r[n1]=out_r[n1]+tr;
                 if(out_r[n1]>15000 || out_r[n1]<-15000){check=1;}   //check for int size, it can handle only +31000 to -31000,
          
                 out_im[n1+i10]=out_im[n1]-ti;
                 out_im[n1]=out_im[n1]+ti;
                 if(out_im[n1]>15000 || out_im[n1]<-15000){check=1;}          
          
                 n1=n1+i10+i10;
                  }       
             }

    if(check==1){                                             // scalling the matrics if value higher than 15000 to prevent varible from overflowing
                for(int i=0;i<a;i++)
                    {
                     out_r[i]=out_r[i]>>1;           
                     out_im[i]=out_im[i]>>1; 
                    }
                     check=0; 
                     scale=scale-1;                 // tracking overall scalling of input data
                }           

     }


if(scale>128)
    {scale=scale-128;
     for(int i=0;i<a;i++)
      {out_r[i]=out_r[i]>>scale;
       out_im[i]=out_im[i]>>scale;
      }
      scale=0;
    }                                                   // revers all scalling we done till here,
else{scale=128-scale;}                             // in case of nnumber getting higher than 32000, we will represent in as multiple of 2^scale

/*
for(int i=0;i<a;i++)
{
Serial.print(out_r[i]);Serial.print("\t");                    // un comment to print RAW o/p    
Serial.print(out_im[i]); 
Serial.print("i");Serial.print("\t"); 
Serial.print("*2^");Serial.println(scale); 
}
*/

//---> here onward out_r contains amplitude and our_in conntains frequency (Hz)
int fout,fm,fstp;
float fstep;
fstep=Frequency/N;
fstp=fstep;
fout=0;fm=0;

    for(int i=1;i<Pow2[o-1];i++)               // getting amplitude from compex number
        { 
              out_r[i]=fastRSS(out_r[i],out_im[i]);
   // Approx RSS function used to calculated magnitude quickly
        
out_im[i]=out_im[i-1]+fstp;
if (fout<out_r[i]){fm=i; fout=out_r[i];}
         /*
         // un comment to print Amplitudes (1st value (offset) is not printed)
         Serial.print(out_r[i]); Serial.print("\t"); 
         Serial.print("*2^");Serial.println(scale); 
         */ 
        }


float fa,fb,fc;
fa=out_r[fm-1];
fb=out_r[fm]; 
fc=out_r[fm+1];
fstep=(fa*(fm-1)+fb*fm+fc*(fm+1))/(fa+fb+fc);

return(fstep*Frequency/N);
}

//---------------------------------fast sine/cosine---------------------------------------//

int fast_sine(int Amp, int th)
{
int temp3,m1,m2;
byte temp1,temp2, test,quad,accuracy;
accuracy=6;    // set it value from 1 to 7, where 7 being most accurate but slowest
               // accuracy value of 5 recommended for typical applicaiton
while(th>1024){th=th-1024;}   // here 1024 = 2*pi or 360 deg
while(th<0){th=th+1024;}
quad=th>>8;

       if(quad==1){th= 512-th;}
  else if(quad==2){th= th-512;}
  else if(quad==3){th= 1024-th;}

temp1= 0;
temp2= 128;     //2 multiple
m1=0;
m2=Amp;

    temp3=(m1+m2)>>1;
    Amp=temp3;
      for(int i=0;i<accuracy;i++)
        { test=(temp1+temp2)>>1;
          temp3=temp3>>1; 
          if(th>isin_data[test]){temp1=test; Amp=Amp+temp3; m1=Amp;}
          else if(th<isin_data[test]){temp2=test; Amp=Amp-temp3; m2=Amp;}
        }

         if(quad==2){Amp= 0-Amp;}
    else if(quad==3){Amp= 0-Amp;}
return(Amp);
}

int fast_cosine(int Amp, int th)
  {  
  th=256-th;  //cos th = sin (90-th) formula
  return(fast_sine(Amp,th)); 
  }

//--------------------------------------------------------------------------------//


//--------------------------------Fast RSS----------------------------------------//
int fastRSS(int a, int b)
{ if(a==0 && b==0){return(0);}
  int min,max,temp1,temp2;
  byte clevel;
  if(a<0){a=-a;}
  if(b<0){b=-b;}
clevel=0;
if(a>b){max=a;min=b;} else{max=b;min=a;}

  if(max>(min+min+min))
    {return max;}
  else
    {
     temp1=min>>3; if(temp1==0){temp1=1;}
     temp2=min;
     while(temp2<max){temp2=temp2+temp1;clevel=clevel+1;}
     temp2=RSSdata[clevel];temp1=temp1>>1;  
     for(int i=0;i<temp2;i++){max=max+temp1;}
     return(max);
    }
}
//--------------------------------------------------------------------------------//










