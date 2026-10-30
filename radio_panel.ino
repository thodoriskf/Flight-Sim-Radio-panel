// Extra Ergasia IoT
//Thodoris Kafetzoglou AM:1117202300085
//Alexandros Gavrilis  AM:1117202100029


//Liquid Crystal kai wire gia thn othoni [2]
//Keypad gia ta pliktra [1]
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>


// LCD (I2C) se address 0x27 16 stiles kai 2 grammes [2]
//den janallazw th dieuthinsi se 0xF, eidame ti egine.
LiquidCrystal_I2C lcd(0x27, 16, 2);


//- - -PARAKATW TA PIN DEFINITIONS- - -


// Gia to Rotary Encoder ta Pins
//CLK -> S1, DT -> S2,SW -> Key 
const int ENCODER_CLK = 6;
const int ENCODER_DT  = A0;
const int ENCODER_SW  = A1;


// Keypad Pins (ta allazoume apo A4/A5 logw tou I2C) [1]
//orizoume 4x4 stilesxgrammes
// arrays gia ta pins gia tis grammes kai stiles, me 8 bit dedomena
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {2, 3, 4, 5};
byte colPins[KEYPAD_COLS] = {10, 9, 8, 7};


// - - - - - - - - - - - - - - - - 



// Keypad character map [1]


char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};


//GLOBAL STATE
//Onoma: customKeypad. Layout:opws sthn metavliti keys.
Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);


// Enumerator Data type giati chars den doulevoun swsta(?) edw
// Metavliti currentMode typou Mode, initialized se radio
enum Mode { RADIO, TRANSPONDER };
 Mode currentMode = RADIO; 


// Enumerator Data type: Radio (gia com kai nav)
// currentRadio typou Radio, initialized se com1
// Pointer gia ta enum values
enum Radio { COM1, COM2, NAV1, NAV2 };
 Radio currentRadio = COM1;
const char* radioNames[] = {"COM1", "COM2", "NAV1", "NAV2"}; 


// Radio Frequencies arxikes
 float com1_active = 118.500, com1_standby = 121.750;
  float com2_active = 119.000, com2_standby = 122.800;
 float nav1_active = 110.50,  nav1_standby = 109.30;
 float nav2_active = 111.20,  nav2_standby = 112.90;


// Transponder ARXIKA
 String transponderCode = "1200";
 String transponderInput = "";


// Encoder state Arxika
 int Encoder_Pos = 0;
 int Last_Encoder_Pos = 0;


//Synartisi Transponder DIGIT VALIDATION gia periorismo se max 7
// den tin kalw ektos transponder mode gt tha ginei xamos
bool isValidXpdrDigit(char k) {
     return (k >= '0' && k <= '7');  // mono octal
}


//SETUP THODORIS
void setup() {
  Serial.begin(9600);
  Serial.println("Flight Panel Initialized (v1.2)");


  lcd.init();
  lcd.backlight();


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Flight Panel");
  lcd.setCursor(0, 1);
  lcd.print("Version 1.4.1");


  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT,  INPUT_PULLUP);
  pinMode(ENCODER_SW,  INPUT_PULLUP);


  delay(2000);
  updateDisplay();
}


//MAIN LOOP THODORIS
void loop() {
  checkEncoder();
  checkKeypad();
}


// Coarse kai Fine an SW pathmeno h oxi antistoixa
void checkEncoder() { 
   int clk_State = digitalRead(ENCODER_CLK); //(HIGH h LOW) apo pin --> clk_State variable.
   int dt_State  = digitalRead(ENCODER_DT); //S2 - -> dt_state variable.
// to parakatw prepei na to allaksw me sinartisi alla doulevei
  if (clk_State == LOW) { // AN metakinithike to encoder
      if (dt_State == HIGH) Encoder_Pos--; // AN metakinithike pros to meion
       else                 Encoder_Pos++;


       if (Encoder_Pos != Last_Encoder_Pos) { // AN exei diaforetiko position apo prin
        if (currentMode == RADIO) { //AN eimaste sto radio mode
         bool fineTuning = (digitalRead(ENCODER_SW) == LOW);  // patimeno = FINE tuning
         changeFrequency(Encoder_Pos > Last_Encoder_Pos, fineTuning); 
         //synartisi me info: fora strofis bool (Encoder_Pos > Last_Encoder_Pos) kai FINEtune.
       }
      Last_Encoder_Pos = Encoder_Pos; //alliws den allakse kati
     }
    delay(1); //debounce
  }


}


//KEYPAD meros
//kalei (customKeypad) an paththei koumpi. If key pressed -> kalese handle me key param
void checkKeypad() {
  char key = customKeypad.getKey();
   if (key) handleKeyPress(key);
}


void handleKeyPress(char key) {
//dexetai (key) xarakthra ws input parameter.


  //RADIO MODE [3]
  if (currentMode == RADIO) {
    switch (key) {
      case 'A':  // Swap active kai standby
        swapFrequencies();
        break;


      case 'B':  // Cycle ta radios
        currentRadio = (Radio)((currentRadio + 1) % 4); 
        //Modulo (% 4) => after NAV2 (index 3), loop ksana sto COM1 (index 0).
        break;


      case 'C':  // allagi se XPDR
        currentMode = TRANSPONDER;
        transponderInput = ""; //reset tou xpdr input
        break;
    }


    updateDisplay(); //kalw tin ananewsi othonis
    return;
  }


  //TRANSPONDER MODE
  if (currentMode == TRANSPONDER) { //an eimai sto xprd


    if (isValidXpdrDigit(key)) { //einai octal? kalw thn prohgoumenh sinartisi me key param
       if (transponderInput.length() < 4)
        transponderInput += key;
    }


    else if (key == '#') {  // Confirm
      if (transponderInput.length() == 4) { //mono confirm an exw symplirwsei ta 4 psifia


        bool ok = true;
        for (byte i = 0; i < 4; i++) {
             if (!isValidXpdrDigit(transponderInput[i])) ok = false;
	// kalei isValidXpdrDigit() gia ton xarakthra sto index (i) tou transponderInput string.
        }


        if (ok) { //an tirithikan ta anwtero
          transponderCode = transponderInput; //ekxwrisi
          Serial.print("XPDR_SET:"); //outoput gia flightsim
          Serial.println(transponderCode);
        }  else {
          Serial.println("XPDR_ERROR:INVALID_DIGIT");
        }
      }


      transponderInput = "";
      currentMode = RADIO;
    }


    else if (key == '*') {  // Clear tou pediou
      transponderInput = "";
    }


    else if (key == 'C') {  // akyrwsh
      transponderInput = "";
      currentMode = RADIO;
    }


    updateDisplay();
  }
}


// Allagi SYXNOTHTAS
// fineTuning = true  -> fine step
// fineTuning = false -> coarse step
void changeFrequency(bool increase, bool fineTuning) { //bool params: i einai i den einai set
  float stepValue;


  if (currentRadio == NAV1 || currentRadio == NAV2) {
    // NAV fine/coarse
    stepValue = fineTuning ? 0.05 : 1.0;
  } else {
    // COM fine/coarse
    stepValue = fineTuning ? 0.025 : 1.0;
  }


  float* standbyFreq; //orismos pointer


  switch (currentRadio) {
    case COM1: standbyFreq = &com1_standby; break;
    case COM2: standbyFreq = &com2_standby; break;
    case NAV1: standbyFreq = &nav1_standby; break;
    case NAV2: standbyFreq = &nav2_standby; break;
  }
//glitwnw polla if statements [4]
  if (increase) *standbyFreq += stepValue;
  else          *standbyFreq -= stepValue;


  updateDisplay();
}


// --- SWAP ACTIVE/STANDBY ---
void swapFrequencies() {
  float temp;


  switch(currentRadio) {
    case COM1:
      temp = com1_active; com1_active = com1_standby; com1_standby = temp;
      break;
    case COM2:
      temp = com2_active; com2_active = com2_standby; com2_standby = temp;
      break;
    case NAV1:
      temp = nav1_active; nav1_active = nav1_standby; nav1_standby = temp;
      break;
    case NAV2:
      temp = nav2_active; nav2_active = nav2_standby; nav2_standby = temp;
      break;
  }


  updateDisplay();
}


// DISPLAY THODORIS
void updateDisplay() {
  lcd.clear(); //katharism lsd
  lcd.setCursor(0, 0);


  if (currentMode == RADIO) { //Ananeonei to display analoga to mode selected
    float active, standby;


    switch(currentRadio) {
      case COM1: active = com1_active; standby = com1_standby; break;
      case COM2: active = com2_active; standby = com2_standby; break;
      case NAV1: active = nav1_active; standby = nav1_standby; break;
      case NAV2: active = nav2_active; standby = nav2_standby; break;
    }


    // Ypologizoume an COARSE h Fine analoga an to SW pressed
    bool fineTuning = (digitalRead(ENCODER_SW) == LOW);
    char stepChar = fineTuning ? 'F' : 'C'; // Sets Fine or Coarse


    lcd.print(radioNames[currentRadio]);
    lcd.print(" ");
    lcd.print(stepChar);
    lcd.print(" ");
    lcd.print("A:"); //Label gia Active Freq
    lcd.print(formatFrequency(active, currentRadio >= NAV1)); //Prints active


    lcd.setCursor(0, 1); // Moves cursor to the start of the second line.
    lcd.print("STBY: ");
    lcd.print(formatFrequency(standby, currentRadio >= NAV1));
    lcd.print(" <");
  }


  else {
    lcd.print("XPDR: "); //XDPR
    lcd.print(transponderCode);
    lcd.setCursor(0, 1);
    lcd.print("NEW: ");
    lcd.print(transponderInput);


    if (millis() % 1000 < 500 && transponderInput.length() < 4) //Blinking cursor
      lcd.print("_");
  }
}


//FORMATTER THODORIS
String formatFrequency(float freq, bool isNav) {
  char buffer[8];
  if (isNav) dtostrf(freq, 6, 2, buffer); // Checks if the radio type is NAV (Navigation).If yes convert to string [5]
  else       dtostrf(freq, 7, 3, buffer); // If COM: Convert float to string gia akriveia   return String(buffer);  // Epistrefei to buffer se Arduino string object. Paradojos Doulevei gia kapoio magiko logo 
}

//[1]: electronicwings.com/arduino/4x4-keypad-interfacing-with-arduino-uno
//[2]: www.youtube.com/watch?v=OC8VC-oz6zA
//[3]: github.com/IWILZ/FS2020-LCD-Panel/blob/main/LCD_and_CDI_1.0.ino
//[4]: forum.arduino.cc/t/understanding-pointers/282746
//[5]: www.programmingelectronics.com/dtostrf/

