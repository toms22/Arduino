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

// promenne
int RealTime = 0;                       // realne mereny cas v sekundach
int StartTime = 0;                      // cas startu, vychazi z millis()
bool TimerRunning = false;              // flag stopek, spusteno/nespusteno
int DisplayRefreshPeriod = 10;          // interval pro aktualizaci displeje (10 ms) -> 100 Hz
int DigitValues[3] = {0, 0, 0};         // cisla k vykresleni na 7seg
int DisplayBrightness = 128;            // jas PWM (0-255)
unsigned long LastDisplayUpdate = 0;
bool bIncrement = true;

int inSerialData;

// Tabulka pro segmenty - zobrazeni znaku 0-9
const uint8_t DigitSegments[10] = {
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

// deklarace funkci
void UpdateTime(void);  
void SplitTimeToDigits(void);
void RefreshDigit(int digit, int value, bool showDot);
void RefreshDisplay(void);
void CheckButtons(void);
void UpdateSerialTime(void);
void CheckSerial(void);

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
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

// the loop function runs over and over again forever
void loop() {

  CheckSerial();            // Obsluha seriove linky
 
  CheckButtons();           // Kontrola tlacitek a vstupnich periferek
  UpdateTime();             // Aktualizace mereneho casu

  // multiplexovani displeje s periodou 10 ms -> 100 Hz
  if (millis() - LastDisplayUpdate >= DisplayRefreshPeriod) {
      LastDisplayUpdate = millis();
      RefreshDisplay();    // Aktualizace displeje 7seg
  }

  // aktualizace casu pres serial
  UpdateSerialTime();     
}


// DEFINICE funkci
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Aktualizace casu stopek
// -------------------------------------------------------------------------------------------------
void UpdateTime(void) {
  // vykona se pouze pokud je flag running splnen
  if (TimerRunning) {
      RealTime = (millis() - StartTime) / 1000; // vysledek je cas v sekundach
  }
}

// -------------------------------------------------------------------------------------------------
// Rozdeleni casu na jednotlive cislice
// -------------------------------------------------------------------------------------------------
void SplitTimeToDigits() {
    if(RealTime == 0){
      // clear values
      DigitValues[2] = 0;
      DigitValues[1] = 0;
      DigitValues[0] = 0;
    }
  
    DigitValues[2] = RealTime / 60;          // minuty
    DigitValues[1] = (RealTime % 60) / 10;   // desitky sekund
    DigitValues[0] = RealTime % 10;           // sekundy
}

// -------------------------------------------------------------------------------------------------
// Zobrazení cislice na 7seg
// -------------------------------------------------------------------------------------------------
void RefreshDigit(int digit, int value, bool showDot) {
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
void RefreshDisplay() {
    // rozdeleni casu do jednotlivych cislic
    SplitTimeToDigits();
    
    // zapsani cislice na prislusne misto - sekundy
    RefreshDigit(Digit1, DigitValues[0], false); 
    delayMicroseconds(DisplayRefreshPeriod);
    
    // zapsani cislice na prislusne misto - desitky sekund
    RefreshDigit(Digit2, DigitValues[1], false); 
    delayMicroseconds(DisplayRefreshPeriod);
    
    // zapsani cislice na prislusne misto - minuty
    RefreshDigit(Digit3, DigitValues[2], true); 
    delayMicroseconds(DisplayRefreshPeriod);
}

// -------------------------------------------------------------------------------------------------
// Obsluha tlacitek
// -------------------------------------------------------------------------------------------------
void CheckButtons() {
  
  // adc hodnota prevodu
  int sensorValue = analogRead(AdcTrim);
  // prevod 10bit adc hodnoty na 8bitovou hodnotu
  DisplayBrightness = map(sensorValue, 0, 1023, 0, 255);

  
  
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
// Posilani casu na serial
// -------------------------------------------------------------------------------------------------
void UpdateSerialTime(){
  
  // interval pro zasilani serial casu
  static unsigned long previousMillis = 0;
  if(!previousMillis) previousMillis = millis();

  // Kontrola, zda uplynul nastaveny interval 1000ms
  if (millis() - previousMillis >= 1000) {
    previousMillis = millis();    // Aktualizace casu

    // Serial print
    Serial.print("Aktualni cas ");
    if(bIncrement){
      Serial.print("inkrementace: ");
    }
    else{
      Serial.print("dekrementace: ");
    }
    // print time
    Serial.print(DigitValues[2]);
    Serial.print(":");
    Serial.print(DigitValues[1]);
    Serial.println(DigitValues[0]);
  }
}

// -------------------------------------------------------------------------------------------------
// Obsluha serialu
// -------------------------------------------------------------------------------------------------
void CheckSerial(){
  if (Serial.available()) {
    inSerialData = Serial.parseInt();
    Serial.println(inSerialData);

    switch(inSerialData){
      case 0:
        Serial.println("RESET");
         TimerRunning = false;
        RealTime = 0;         // clear timer
        break;

      case 1:
        Serial.println("START");
        StartTime = millis();
        TimerRunning = true;
        break;

      case 2:
        Serial.println("STOP");
        TimerRunning = false;
        break;
       
      default:
        break;
    }
  }
}
