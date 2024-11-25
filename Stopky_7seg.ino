// Autor TomHam
// ArduinoIDE v 1.8.13

// -------------------------------------------------------------------------------------------------
// pro tvoje zapojeni zakomentovat tento define !!!
#define INPUT_TESTING_PULLUP
// -------------------------------------------------------------------------------------------------

// konstanty, nahrada #define
// tlacitka a jine vstupy
const int AdcTrim = A0;       					// adc hodnota trimru

// pocet tlacitek
const int NumberOfButtons = 4;				
// zapojeni jednotlivych talcitek	na piny 15, 16, 17, 18
const int BtnPins[NumberOfButtons] = {15, 16, 17, 18};

// jednotlive segmenty
const int SegA = 8; 
const int SegB = 2; 
const int SegC = 3; 
const int SegD = 4; 
const int SegE = 5; 
const int SegF = 6; 
const int SegG = 7;
const int SegDP = 12;

// jednotlive anody
const int Digit1 = 11;    // sekundy
const int Digit2 = 10;    // desitky sekund
const int Digit3 = 9;    	// minuty
// pocet digitu
const int NumberOfDigits = 3;

// Debounce talcitka cas v ms
const unsigned long DebounceDelay = 50;
// Krok odecteni/pricteni casu v sekundach
const int TimeStep = 30;
// Limitni cas kdy dojde k blikani displeje v sekundach
const int BlinkingTime = 15;
// Perioda blikani displeje v milisekundach
const int BlinkingPeriod = 200;

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
// globalni promenne
int RealCounter = 0;                       	// realne mereny cas v sekundach
int SetCounter = 180;												// ulozeny vychozi cas v sekundach
bool TimerRunning = false;              		// flag stopek, spusteno/nespusteno
int DigitValues[NumberOfDigits];         		// cisla k vykresleni na 7seg
bool bIncrement = false;										// nastaveni pricitani/odcitani casu, defaultne odcitani
int DisplayBright_DutyCycle;      					// jas PWM (0-100)
bool bDisplayON = true;											// flag pro blikani displeje, kdyz je mereny cas pod hodnotou BlinkingTime

// Pole pro sledovani stavu tlacitek
bool CurrentBtnState[] = {false, false, false, false};
bool LastBtnState[] = {false, false, false, false};
unsigned long LastDebounceTime[] = {0, 0, 0, 0};

// -------------------------------------------------------------------------------------------------
// deklarace funkci
void UpdateTime(void);  
void SplitTimeToDigits(void);
void SetDigit(int digit, int value, bool showDot);
void RefreshDisplay_7seg(int RefreshPeriod, int DutyCycle);
void AnodeAllOFF(void);
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

  // nastaveni tlacitek, pull ups
  for(int i = 0; i < NumberOfButtons; i++)
  {
    // nastaveni pullups 
#ifdef INPUT_TESTING_PULLUP
    pinMode(BtnPins[i], INPUT_PULLUP);
#else
    pinMode(BtnPins[i], INPUT);
#endif
  }

  // nacteni vychoziha casu 5 minut
  RealCounter = SetCounter;
}
// -------------------------------------------------------------------------------------------------
// main loop
void loop() 
{	
  // Kontrola tlacitek a vstupnich periferek
  CheckButtons();   

  // Aktualizace mereneho casu
  UpdateTime();	

  // Obsluha seriove linky
  SerialTask();   

  // Refresh displeje s nastavenou periodou a svitivosti
  RefreshDisplay_7seg(10, DisplayBright_DutyCycle);
}

// definice funkci
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Aktualizace casu stopek
// -------------------------------------------------------------------------------------------------
void UpdateTime(void) 
{	
  // casovac pro update casu
  static unsigned long UpdaterTimer = 0;
  // inicializace pri prvnim volani
  if(!UpdaterTimer) UpdaterTimer = millis();

  // update casu po 1000 ms
  if (millis() - UpdaterTimer >= 1000) 
  {
    UpdaterTimer = millis();    // Aktualizace casu casovace pro dalsi cyklus

    // pokud timer bezi
    if(TimerRunning == true) 
    {			
      // inkrementace counteru
      if(bIncrement == true)
      {
        RealCounter++;
      }

      // dekrementace counteru
      else
      {
        RealCounter--;

        // osetreni zapornych hodnot, timer je na nule
        if(RealCounter < 0)
        {				
          // nastaveni ulozeneho casu
          RealCounter = SetCounter;
          // nastaveni dekrementace
          bIncrement = false;
          // vypnuti timeru
          TimerRunning = false;
          // trvale zapnuti displeje
          bDisplayON = true;
        }
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
  // casovace pro multiplex displeje a blikani displeje
  static unsigned long MuxTimer = 0;
  static unsigned long BlinkTimer = 0;
  // inicializace pri prvnim volani
  if(!MuxTimer) MuxTimer = millis();
  if(!BlinkTimer) BlinkTimer = millis();

  // casovac pro blikani displeje v pozadovane periode a pokud je mereny cas pod stanovenou uroven
  if((millis() - BlinkTimer >= BlinkingPeriod) && (RealCounter <= BlinkingTime))
  {
    // aktualizace casu
    BlinkTimer = millis();
    // obraceni flagu pro blikani
    bDisplayON = !bDisplayON;
  }

  // multiplexovani displeje s nastavenou periodou
  if (millis() - MuxTimer >= RefreshPeriod) 
  {
    MuxTimer = millis();    // Aktualizace casu casovace pro dalsi cyklus

    // rozdeleni casu na jednotlive cislice
    SplitTimeToDigits();

    // kontrola limitu dyty cycle 5-100 %
    if(DutyCycle <= 5) DutyCycle = 5;
    if(DutyCycle >= 100) DutyCycle = 100;

    // vypocet duty cycle pro svit displeje
    int Delay = (RefreshPeriod * DutyCycle * 10) / NumberOfDigits;

    // zapnuty displej
    if(bDisplayON == true)
    {
      // zapsani cislice na prislusne misto - sekundy
      SetDigit(Digit1, DigitValues[0], false); 
      // zpozdeni vypocteny cas podle DutyCycle
      delayMicroseconds(Delay);
      // vsechny anody OFF
      AnodeAllOFF();

      // zapsani cislice na prislusne misto - desitky sekund
      SetDigit(Digit2, DigitValues[1], false); 
      delayMicroseconds(Delay);
      AnodeAllOFF();

      // zapsani cislice na prislusne misto - minuty
      SetDigit(Digit3, DigitValues[2], true); 
      delayMicroseconds(Delay);
      AnodeAllOFF();
    }

    // vypnuty displej, vsechno OFF
    else
    {
      AnodeAllOFF();
    }
  }
}

// -------------------------------------------------------------------------------------------------
// Vsechny anody OFF
// -------------------------------------------------------------------------------------------------
void AnodeAllOFF(void)
{
  // vsechny anody OFF
  digitalWrite(Digit1, LOW); 
  digitalWrite(Digit2, LOW);
  digitalWrite(Digit3, LOW); 
}

// -------------------------------------------------------------------------------------------------
// Obsluha tlacitek
// -------------------------------------------------------------------------------------------------
void CheckButtons()
{
  // obsluha ADC
  // adc hodnota prevodu
  int sensorValue = analogRead(AdcTrim);
  // prevod 10bit adc hodnoty na procenta svitu displeje 0-100
  DisplayBright_DutyCycle = map(sensorValue, 0, 1023, 0, 100);

  // obsluha tlacitek
  unsigned long CurrentMillis = millis();

  // cteni a zpracovani vsech tlacitek
  for (int i = 0; i < NumberOfButtons; i++) 
  {
    // cteni prislusneho tlacitka
#ifdef INPUT_TESTING_PULLUP
    bool ReadState = !digitalRead(BtnPins[i]); 		// spinani tlacitkem GND
#else
    bool ReadState = digitalRead(BtnPins[i]); 		// spinani tlacitkem VDD
#endif

    // pokud se stav tlacitka zmenil, ulozime aktualni cas
    if (ReadState != LastBtnState[i]) 
    {
      // ulozeni casu posledni zmeny
      LastDebounceTime[i] = CurrentMillis; 
    }

    // kontrola debounce time
    if ((CurrentMillis - LastDebounceTime[i]) > DebounceDelay) 
    {
      // pokud je stav stabilni a odlisny od minuleho
      if (ReadState != CurrentBtnState[i]) 
      {
        // aktualizujeme stav
        CurrentBtnState[i] = ReadState;

        // akce pri stisku prislusnych tlacitek
        if (CurrentBtnState[i]) 
        {					
          // zpracovani prislusne akce prislusneho tlacitka
          switch(i)
          {
            // -------------------
            // tlacitko start -> A1
            case 3:
              // tisk requestu na monitor
              Serial.println("*** START ***");
              // ulozeni nastavene hodnoty
              SetCounter = RealCounter;
              // spusteni timeru
              TimerRunning = true;
              break;

            // -------------------
            // tlacitko minus -> A2
            case 2:
              // tisk requestu na monitor
              Serial.println("*** MINUS ***");
              // odecteni definovaneho casu
              RealCounter -= TimeStep;
              // limity min
              if(RealCounter <= 30) RealCounter = 30;
              break;

            // -------------------
            // tlacitko plus -> A3
            case 1:
              // tisk requestu na monitor
              Serial.println("*** PLUS ***");
              // pristeni definovaneho casu
              RealCounter += TimeStep;
              // limity max
              if(RealCounter >= 300) RealCounter = 300;
              break;

            // -------------------
            // tlacitko reset -> A4
            case 0:
              // tisk requestu na monitor
              Serial.println("*** RESET ***");
              // nacteni ulozene hodnoty do ziveho counteru
              RealCounter = SetCounter;
              // nastaveni odcitani, pro jistotu
              bIncrement = false;
              // vypnuti timeru
              TimerRunning = false;
              // trvale zapnuti displeje
              bDisplayON = true;
              break;

            default:
              break;

          }
        }
      }
    }
    // aktualizace posledního cteni
    LastBtnState[i] = ReadState;
  }
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
  if ((millis() - SerialTimer >= 1000) && (TimerRunning == true)) 
  {
    SerialTimer = millis();    // Aktualizace casu casovace pro dalsi cyklus

    // Serial print
    Serial.print("Cas ");

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
        Serial.println("*** RESET ***");
        RealCounter = SetCounter;
        bIncrement = false;
        TimerRunning = false;
        // trvale zapnuti displeje
        bDisplayON = true;
        break;

      // -------------------
      // start
      case '1':
        Serial.println("*** START ***");
        SetCounter = RealCounter;
        TimerRunning = true;
        break;

      // -------------------
      // stop/pause
      case '2':
        Serial.println("*** STOP ***");
        TimerRunning = false;
        break;

      // -------------------
      // inkrementace casu
      case 'I':
      case 'i':
        Serial.println("*** INKREMENTACE ***");
        bIncrement = true;
        break;

      // -------------------
      // dekrementace casu
      case 'D':
      case 'd':
        Serial.println("*** DEKREMENTACE ***");
        bIncrement = false;
        break;				

      case '+':
        Serial.println("*** PLUS ***");
        RealCounter += TimeStep;
        // limity max
        if(RealCounter >= 300) RealCounter = 300;
        break;

      case '-':
        Serial.println("*** MINUS ***");
        RealCounter -= TimeStep;
        // limity min
        if(RealCounter <= 30) RealCounter = 30;
        break;

      default:
        break;
    }
  }
}
