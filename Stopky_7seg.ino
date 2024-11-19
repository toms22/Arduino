// Autor TomHam
// ArduinoIDE v 1.8.13

// -------------------------------------------------------------------------------------------------
// konstanty, nahrada #define
// tlacitka a jine vstupy
const int BtnRez = A1;        // Rezerva
const int BtnInc = A2;        // inkrementace
const int BtnDec = A3;        // dekrementace
const int BtnReset = A4;      // reset
const int AdcTrim = A0;       // adc hodnota trimru

// jednotlive segmenty
const int SegA = 1; 
const int SegB = 5; 
const int SegC = 6; 
const int SegD = 7; 
const int SegE = 8; 
const int SegF = 9; 
const int SegG = 10;
const int SegDP = 11;

// jednotlive anody
const int Digit1 = 12;    // sekundy
const int Digit2 = 13;    // desitky sekund
const int Digit3 = 14;    // minuty

// pocet digitu
const int NumberOfDigits = 3;

// Tabulka pro segmenty - zobrazeni znaku 0-9
const uint8_t DigitSegments[] = {
    0b1111110, // 0
    0b0110000, // 1
    0b1101101, // 2
    0b1111001, // 3
    0b0110011, // 4
    0b1011011, // 5
    0b1011111, // 6
    0b1110000, // 7
    0b1111111, // 8
    0b1111011  // 9
};

// -------------------------------------------------------------------------------------------------
// promenne
int RealCounter = 0;                       	// realne mereny cas v sekundach
bool TimerRunning = false;              		// flag stopek, spusteno/nespusteno
int DigitValues[NumberOfDigits];         		// cisla k vykresleni na 7seg
bool bIncrement = true;											// nastaveni pricitani/odcitani casu
int DisplayBright_DutyCycle;      					// jas PWM (0-100)

// -------------------------------------------------------------------------------------------------
// deklarace funkci
void UpdateTime(void);  
void SplitTimeToDigits(void);
void SetDigit(int digit, int value, bool showDot);
void RefreshDisplay_7seg(int RefreshPeriod, int DutyCycle);
void CheckButtons(void);
void SerialTask(void);

// -------------------------------------------------------------------------------------------------
// setup periferek
void setup() 
{
  // init interni ledky
  pinMode(LED_BUILTIN, OUTPUT);
  
  // init Seg
  pinMode(SegA, OUTPUT); 
  pinMode(SegB, OUTPUT);
  pinMode(SegC, OUTPUT); 
  pinMode(SegD, OUTPUT);
  pinMode(SegE, OUTPUT); 
  pinMode(SegF, OUTPUT);
  pinMode(SegG, OUTPUT); 
  pinMode(SegDP, OUTPUT);

  // init Anode
  pinMode(Digit1, OUTPUT); 
  pinMode(Digit2, OUTPUT);
  pinMode(Digit3, OUTPUT);

  // init serial 9600 BaudRate
  Serial.begin(9600);
}
// -------------------------------------------------------------------------------------------------
// main loop
void loop() 
{
	// Obsluha seriove linky
  SerialTask();            
	
	// Kontrola tlacitek a vstupnich periferek
  CheckButtons();   
	
	// Aktualizace mereneho casu
  UpdateTime();	
	
	// Refresh displeje s nastavenou periodou a svitivosti
	RefreshDisplay_7seg(10, 50);
	//RefreshDisplay_7seg(10, DisplayBright_DutyCycle);
}


// definice funkci
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Aktualizace casu stopek
// -------------------------------------------------------------------------------------------------
void UpdateTime(void) 
{	
	// pokud timer bezi
	if(TimerRunning == true) 
	{
		// inkrement kazdou sekundu
		if((millis() % 1000) == 0)
		{
			// rozsviceni ledky pro kontrolu periody
			digitalWrite(LED_BUILTIN, HIGH);
			delay(10); 
			digitalWrite(LED_BUILTIN, LOW);
			
			// inkrementace counteru
			if(bIncrement == true)
			{
				RealCounter++;
			}
			
			// dekrementace counteru
			else
			{
				RealCounter--;
				
				// osetreni zapornych hodnot
				if(RealCounter <= 0)
					RealCounter = 0;
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------
// Rozdeleni casu na jednotlive cislice
// -------------------------------------------------------------------------------------------------
void SplitTimeToDigits() 
{
	DigitValues[2] = RealCounter / 60;          // minuty
	DigitValues[1] = (RealCounter % 60) / 10;   // desitky sekund
	DigitValues[0] = RealCounter % 10;          // sekundy
}

// -------------------------------------------------------------------------------------------------
// Zobrazení cislice na 7seg
// -------------------------------------------------------------------------------------------------
void SetDigit(int digit, int value, bool showDot) 
{
	// vsechny anody OFF
	digitalWrite(Digit1, LOW); 
	digitalWrite(Digit2, LOW);
	digitalWrite(Digit3, LOW); 
	
	// prislusna anoda ON - u multiplexu sviti vzdy jen jedna
	digitalWrite(digit, HIGH);
	
	byte segments = DigitSegments[value];
	// maskovani a rozviceneni kokretniho segmentu displeje
	digitalWrite(SegA, segments & B1000000);
	digitalWrite(SegB, segments & B0100000);
	digitalWrite(SegC, segments & B0010000);
	digitalWrite(SegD, segments & B0001000);
	digitalWrite(SegE, segments & B0000100);
	digitalWrite(SegF, segments & B0000010);
	digitalWrite(SegG, segments & B0000001);
	// rozviceni DP podle flagu showDot
	digitalWrite(SegDP, showDot);
}

// -------------------------------------------------------------------------------------------------
// Multiplex dipleje
// -------------------------------------------------------------------------------------------------
void RefreshDisplay_7seg(int RefreshPeriod, int DutyCycle)
{
	// casovac pro multiplex displeje
  static unsigned long MuxTimer = 0;
	// inicializace pri prvnim volani
  if(!MuxTimer) MuxTimer = millis();

  // multiplexovani displeje s nastavenou periodou
  if (millis() - MuxTimer >= RefreshPeriod) 
	{
		MuxTimer = millis();    // Aktualizace casu casovace pro dalsi cyklus
	
		// rozdeleni casu na jednotlive cislice
		SplitTimeToDigits();
		
		// kontrola limitu dyty cycle
		if(DutyCycle <= 10) DutyCycle = 10;
		if(DutyCycle >= 100) DutyCycle = 100;
		
		// vypocet duty cycle pro svit displeje
		int Delay = (RefreshPeriod * DutyCycle * 10) / NumberOfDigits - 1;
		
		// zapsani cislice na prislusne misto - sekundy
		SetDigit(Digit1, DigitValues[0], false); 
		delayMicroseconds(Delay);
		
		// zapsani cislice na prislusne misto - desitky sekund
		SetDigit(Digit2, DigitValues[1], false); 
		delayMicroseconds(Delay);
		
		// zapsani cislice na prislusne misto - minuty
		SetDigit(Digit3, DigitValues[2], true); 
		delayMicroseconds(Delay);
	}
}

// -------------------------------------------------------------------------------------------------
// Obsluha tlacitek
// -------------------------------------------------------------------------------------------------
void CheckButtons()
{
  // adc hodnota prevodu
  int sensorValue = analogRead(AdcTrim);
  // prevod 10bit adc hodnoty na procenta svitu displeje 0-100
  DisplayBright_DutyCycle = map(sensorValue, 0, 1023, 0, 100);

/*
	// --------------------------------------------------------------------
	// cteni tlacitek
	// dodelat jednotnou funkci, ideal pres datovou strukturu
	int DebounceDelay = 50; // Debounce doba v ms
	static bool CurrentButtonState = false; 
	static bool LastButtonState = false; 
	static unsigned long LastTime = 0;
	static unsigned long CurrentTime = millis();
  
	// cteni stavu
	bool ReadBtnState = digitalRead(BtnInc);
	
	// zmena stavu
	if(ReadBtnState != LastButtonState)
	{
		// aktualizace casu zmeny
		LastTime = CurrentTime;
	}
	
	// debounce
  if ((CurrentTime - LastTime) > DebounceDelay) 
	{
    // stabilni zmena 
    if (ReadBtnState != CurrentButtonState) 
		{
			// aktualizace stavu
      CurrentButtonState = ReadBtnState;

      // Vraci stav
      if (CurrentButtonState == true) 
			{
        return true;
      }
    }
  }
	
	// stav posledniho cteni
	LastButtonState = ReadBtnState;
	return false;
	*/
	
  /*
    static bool LastButtonState = HIGH;
    bool ButtonState = digitalRead(buttonPin);

    if (ButtonState == LOW && LastButtonState == HIGH) {
        if (!TimerRunning) {
            StartTime = millis();
            RealTime = 0;
            TimerRunning = true;
        } else {
            TimerRunning = false;
        }
        delay(50); // Debounce
    }
    LastButtonState = ButtonState;
    */
}

// -------------------------------------------------------------------------------------------------
// Obsluha serialu prijem/vysilani
// -------------------------------------------------------------------------------------------------
void SerialTask()
{
	// intervalovy casovac pro zasilani serial casu na seriovy monitor
  static unsigned long SerialTimer = 0;
	// inicializace pri prvnim volani
  if(!SerialTimer) SerialTimer = millis();

  // Kontrola, zda uplynul nastaveny interval 1000ms, 
  if (millis() - SerialTimer >= 1000) {
    SerialTimer = millis();    // Aktualizace casu casovace pro dalsi cyklus

    // Serial print
    Serial.print("Aktualni cas ");
    if(bIncrement){
      Serial.print("inkrementace: ");
    }
    else{
      Serial.print("dekrementace: ");
    }
    // print time, format je stejny jako pro multiplex
    Serial.print(DigitValues[2]);
    Serial.print(":");
    Serial.print(DigitValues[1]);
    Serial.println(DigitValues[0]);
  }
	
	// ---------------------------------------------------
	// kontrola prijeti nove zpravy
  if (Serial.available()) 
	{
		// prijeti zpravy
    int inSerialData = Serial.read();
		
		// prislusna akce na zaklade prijate zpravy
    switch(inSerialData)
		{
			// -------------------
			// reset
      case '0':
        Serial.println("RESET");
        RealCounter = 0;         // clear counter
        break;
			
			// -------------------
			// start
      case '1':
        Serial.println("START");
        TimerRunning = true;
        break;
			
			// -------------------
			// stop/pause
      case '2':
        Serial.println("STOP");
        TimerRunning = false;
        break;
			
			// -------------------
			// inkrementace casu
			case '+':
        Serial.println("INKREMENT");
				bIncrement = true;
        break;
			
			// -------------------
			// dekrementace casu
			case '-':
        Serial.println("DEKREMENT");
				bIncrement = false;
        break;
				
      default:
        break;
    }
  }
}
