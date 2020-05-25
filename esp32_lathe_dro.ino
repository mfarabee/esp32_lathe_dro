// For NodeMCU-32s and 240x320 TFT touch display with SD card
/*
Connections:
DISPLAY          ESP32        NOTES
T_IRQ            NC          Not using IRQ
T_DO             NC
T_DIN            GPIO23
T_CS             GPIO4
T_CLK            GPIO18
SDO(MISO)        GPIO19
LED              3.3V
SCK              GPIO18
SDI(MOSI)        GPIO23
DC               GPIO33
RESET            GPIO27
CS               GPIO05
GND              GND
VCC              +5V (VIN)   The display has a voltage regulator requiring +5V input

SD_CS            GPIO15
SD_MOSI          GPIO13
SD_MISO          GPIO26      No Tristate driver on Display card for SPI interface
SD_SCK           GPIO14



Available: GPIO 2,0,16,17,21,3,1,22,12,32,39,36  (34-39 - input only, no pullup)
)
*/

#include <ESP32Encoder.h> // https://github.com/madhephaestus/ESP32Encoder/
#include "esp32_lathe_dro.h"
#include "tftDisplay.h"
#include "tftSD.h"
#include "keyboard.h"
#include <EEPROM.h>  //https://github.com/espressif/arduino-esp32/tree/master/libraries/EEPROM

TFT_CLASS *Tft;
ESP32Encoder encoder;
KEYLIST_CLASS *DroKeyList,*SetupKeyList;
int CounterMultiplier=0;
int64_t IncrementalOffset=0;
int AbsIncMode=ABSOLUTE;

// The encoder will rollover to zero and continue in positive or negative direction  
// -> 0,1,32766,0,1,...    
// -> 0,-1,-32767,0,-1,...
int64_t CounterOffset(int mult){
  int64_t result;
  if (mult==0){
    result=0;
  }else if(mult > 0){
    result=mult*32767;
  }else{
    result=mult*32768;
  }
  return(result);
}

/*
 * getMyCount
 * Description: Reads encoder value, checks for overflow and returns result 
 */
int64_t getMyCount(int force=0){
  int64_t result;
  static int32_t prevCount=0;
  int32_t raw=0;
  
  raw=encoder.getCountRaw();
  // Because the encoder will roll over after +/- 32767, I need to keep track of how many times
  // 30000 and 2000 are arbitrary, bracketing the rollover
  if (prevCount >30000 && raw <=2000){ // counter (overflow) rolled  from 32766 to 0
    CounterMultiplier += 1;
  } else if(prevCount < -30000 && raw >= -2000){  // counter (overflow) rolled  from -32767 to 0
    CounterMultiplier -=1;
  }
  prevCount=raw;  
  result=raw+CounterOffset(CounterMultiplier);
  if(force != 1 && AbsIncMode == INCREMENTAL){
    result += IncrementalOffset;
  }
  return(result);
}

/*
 * initDRODisplay
 * initialize DRO 
 */
void initDRODisplay(){
  Tft->fillScreen(ILI9341_BLACK);
  DroKeyList->drawKeys();
  drawAbsInc();
  Tft->fillRect(0,ROW[6], Tft->w, DRO_H,ILI9341_BLUE);
  Tft->setTextColor(ILI9341_WHITE); Tft->setCursor(80,ROW[6]);
  Tft->print("Setup");
  displayDRO(1);
}

/*
 * dislaySetup
 * Description: Creates.setup screen
 */
void displaySetup(){
  int32_t stepCnt;
  int swap;
  char value;
  
  swap=SETUP.SWAPX;
  stepCnt=SETUP.STEP_ADJUSTMENT;
  Tft->fillScreen(ILI9341_BLACK);
  SetupKeyList->drawKeys();
  
  Tft->setTextColor(ILI9341_WHITE);
  Tft->setTextSize(DRO_SCALE);
  Tft->setCursor(100,ROW[1]);Tft->print(swap);
  Tft->setCursor(100,ROW[3]);Tft->print(stepCnt);
  
  value=SetupKeyList->checkKeys();
  while (value != 'A' && value !='C'){
    if(value != '\0'){
      Tft->setTextSize(DRO_SCALE);
      switch(value){
        case 'R':  // reverse direction of scale
          swap *= -1;
          Tft->fillRect(100,ROW[1],Tft->w-100,DRO_H,ILI9341_BLACK);
          Tft->setCursor(100,ROW[1]); Tft->print(swap);
        break;
        case '-':  // decrease multiplier count
        stepCnt -= 1;
        Tft->fillRect(100,ROW[3],140,DRO_H,ILI9341_BLACK);
        Tft->setCursor(100,ROW[3]); Tft->print(stepCnt);
        break;
        case '+':  // increase multiplier count
        stepCnt += 1;
        Tft->fillRect(100,ROW[3],140,DRO_H,ILI9341_BLACK);
        Tft->setCursor(100,ROW[3]); Tft->print(stepCnt);
        break;        
        }
    }
    value=SetupKeyList->checkKeys();
  }
  if(value == 'A'){ // Apply settings and save to EEPROM
    SETUP.SWAPX=swap;
    SETUP.STEP_ADJUSTMENT=stepCnt;
    STEPS=ENCODER_PULSE + SETUP.STEP_ADJUSTMENT;
    if(EEPROM_WORKING == 1){
      EEPROM.writeBytes(EEPROM_START_ADDR,&SETUP,EEPROM_SIZE);
      EEPROM.commit();
      //Serial.println("writing to EEPROM");
    }
  }
}



// Displays Coordinates
void displayDRO(int resetDisplay){
  int64_t countX;
  char tmp[100];
  double cXin,cXmm;
  static char prevXmm[20]="",prevXin[20]="";
  /*
   *         ABS/INC
   *      1234.1234 mm
   *      1234.1234 in
   */

   countX=getMyCount();
   cXmm=SETUP.SWAPX*40.0*(countX/(double)STEPS);
   cXin=cXmm/25.4;

  Tft->setTextSize(DRO_SCALE);
  if(resetDisplay==1){
     strcpy(prevXmm,"");strcpy(prevXin,"");
     Tft->fillRect(DRO_STARTX,ROW[2], DRO_DIGITS*DRO_W, DRO_H,ILI9341_BLACK);
     Tft->fillRect(DRO_STARTX,ROW[3],DRO_DIGITS*DRO_W, DRO_H,ILI9341_BLACK);
  }

  sprintf(tmp,"%-5.4f mm",cXmm); 
  if(strcmp(tmp,prevXmm)!=0){ // Only print if different
    // draw old value on background (erase), then draw new value (reduces screen flash)
    Tft->setTextColor(ILI9341_BLACK); Tft->setCursor(DRO_STARTX+(DRO_W*(DRO_DIGITS-strlen(prevXmm))),ROW[2]); Tft->print(prevXmm);
    Tft->setTextColor(ILI9341_YELLOW); Tft->setCursor(DRO_STARTX+(DRO_W*(DRO_DIGITS-strlen(tmp))),ROW[2]); Tft->print(tmp);
    strcpy(prevXmm,tmp);

    sprintf(tmp,"%-5.4f in",cXin); 
    //sprintf(tmp,"%d raw",raw);  // debug
    Tft->setTextColor(ILI9341_BLACK); Tft->setCursor(DRO_STARTX+(DRO_W*(DRO_DIGITS-strlen(prevXin))),ROW[4]); Tft->print(prevXin);
    Tft->setTextColor(ILI9341_YELLOW); Tft->setCursor(DRO_STARTX+(DRO_W*(DRO_DIGITS-strlen(tmp))),ROW[4]); Tft->print(tmp);
    strcpy(prevXin,tmp);
  }
}

/*
 * debounce
 * Description: locks out button press for specified time, This eliminates switch bounce.
 */
int debounce(int pinNum){
  static unsigned long debounce[50]={0L};
  unsigned long debounceTime=0;
  int result=1;
  
  if(debounce[pinNum] == 0L){
    debounce[pinNum]=millis();
    //Serial.println("set");         
  }else {
    debounceTime=(millis()- debounce[pinNum]);
    if(debounceTime < DEBOUNCE_TIME){
      //Serial.print(debounceTime);Serial.print("   ");Serial.println("fail"); 
      result=0;
    }else{
      debounce[pinNum]=millis();
      //Serial.println("set2"); 
    }
  }
  //Serial.print(pinNum);Serial.print("  ");Serial.println(debounce[pinNum]);
  return(result);
}

/*
 * checkSwitches
 * Description: checks all switches and preforms specified function if switch is pressed
 */
void checkSwitches(){
  if(digitalRead(ZERO_PIN)==LOW){
    if(debounce(ZERO_PIN)==1){
      if(AbsIncMode == ABSOLUTE){ // zero absolute
        encoder.clearCount();
        CounterMultiplier=0;
        drawBuzzer(BUZZER_OFF);
      }else{ // zero incremental
        IncrementalOffset= -getMyCount(1);
        drawBuzzer(BUZZER_STATE);
      }    
    }
  }
  if(digitalRead(INC_PIN)==LOW){
    if(debounce(INC_PIN)==1){
        AbsIncMode *= -1;
        //Serial.println(AbsIncMode);
        drawAbsInc();
        drawBuzzer(BUZZER_STATE);
    }
  }
  if(digitalRead(BUZZER_IN_PIN)==LOW){
    if(debounce(BUZZER_IN_PIN)==1){
      if(BUZZER_STATE != BUZZER_OFF){
        drawBuzzer(BUZZER_OFF);
      }else{
        BUZZER_COUNT=getMyCount(1); // get absolute count
        drawBuzzer(BUZZER_WAIT_FOR_RESET);
      }
    }
  }
} 

/*
 * drawAbsInc
 * Description: Draws appropriate text on screen for absolute or increment mode
 */
void drawAbsInc(){
  Tft->setTextSize(DRO_SCALE);
  Tft->fillRect(0,ROW[0], Tft->width(), DRO_H,ILI9341_BLACK);
  Tft->setTextColor(ILI9341_WHITE); Tft->setCursor(40,ROW[0]);
  if(AbsIncMode == ABSOLUTE){
     Tft->print(" ABSOLUTE");
  }else{
    Tft->print("INCREMENTAL");
  } 
}

/*
 * checkKeyboard
 * Description: checks touch screen keylist. Currently the only key is for setup mode.
 */
String checkKeyboard(){
  char value;
  String result="";
  
  value=DroKeyList->checkKeys();
  if(value != '\0'){
    switch(value){
      case 'S':
        displaySetup();
        initDRODisplay();
      break;
      }      
  }
  return(result);
}

/*
 * drawBuzzer
 * Description: display buzzer status when buzzer is active, else it clears the buzzer text
 */
void drawBuzzer(int flag){
  double cXmm;
  char tmp[100];
  
  BUZZER_STATE=flag;
  Tft->setTextSize(DRO_SCALE-1);
  Tft->fillRect(0,ROW[5], Tft->w, DRO_H,ILI9341_BLACK);
  if(AbsIncMode == ABSOLUTE){
    cXmm=SETUP.SWAPX*40.0*(BUZZER_COUNT/(double)STEPS);
  }else{
    cXmm=SETUP.SWAPX*40.0*((BUZZER_COUNT+IncrementalOffset)/(double)STEPS);
  }
  
  sprintf(tmp,"Buzz:%-5.4fmm",cXmm);
  switch(flag){
    case BUZZER_READY: // buzzer active
      Tft->setTextColor(ILI9341_GREEN);
      Tft->setCursor(0,ROW[5]);Tft->print(tmp);
    break;
    case BUZZER_WAIT_FOR_RESET: // buzzer waiting to reset
      Tft->setTextColor(ILI9341_RED);
      Tft->setCursor(0,ROW[5]);Tft->print(tmp);
    break;
    default:
    break;
  }    
}

/*
 * checkBuzzer
 * Description:changes state of buzzer and sounds buzzer when needed
 * Buzzer is active LOW (on when output is LOW)
 */
void checkBuzzer(){
  static int BuzzDirection=0;
  int64_t currentCnt;

  if(BUZZER_STATE != BUZZER_OFF){
    currentCnt=getMyCount(1);

    // if current hits BUZZER_COUNT 
    if(BUZZER_STATE==BUZZER_READY){
      if((BuzzDirection == -1 && currentCnt <= BUZZER_COUNT) ||  // moving to small value
         (BuzzDirection == 1  && currentCnt >= BUZZER_COUNT)){   // moving to larger value
          displayDRO(0); // Need to do this so display matched beep value 
          digitalWrite(BUZZER_OUT_PIN,LOW);
          delay(BUZZER_TIME);
          digitalWrite(BUZZER_OUT_PIN,HIGH);
          drawBuzzer(BUZZER_WAIT_FOR_RESET);
      }
    }
    //  Serial.print((int32_t)abs(currentCnt-BUZZER_COUNT));
    //  Serial.print("  ");Serial.println((int32_t)BUZZER_LIMIT);
      
    // if current has moved outside limit, enable buzzer(1)
    if( (BUZZER_STATE==BUZZER_WAIT_FOR_RESET) && (abs(currentCnt-BUZZER_COUNT)>BUZZER_LIMIT)){
      drawBuzzer(BUZZER_READY);
      if(currentCnt>BUZZER_COUNT){
        BuzzDirection= -1;
      }else{
        BuzzDirection=1;
      }
    }   
  }
}

void setup() {
  String result="";
  SETUP_STRUCT tmp;

  Serial.begin(115200);
  ESP32Encoder::useInternalWeakPullResistors=false;

  pinMode(ZERO_PIN, INPUT_PULLUP);
  pinMode(INC_PIN, INPUT_PULLUP);
  pinMode(BUZZER_IN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_OUT_PIN, OUTPUT);
  digitalWrite(BUZZER_OUT_PIN,HIGH); // turn off buzzer
  
  // Allow different encoder pulse modes. Full is most accurate.
  switch(ENCODER_TYPE){
    case 'S':
      encoder.attachSingleEdge(A_PIN, B_PIN);
      ENCODER_PULSE=600;
    break;
    case 'H':
      encoder.attachHalfQuad(A_PIN, B_PIN);
      ENCODER_PULSE=1200;
    break;
    case 'F':
    default:
      encoder.attachFullQuad(A_PIN, B_PIN);
      ENCODER_PULSE=2400;
    break;
  }
  // Need to define (redefine) encoder pins after attach, to redefine them as pullups to take
  // advantage of internal pullups
  pinMode(A_PIN, INPUT_PULLUP);
  pinMode(B_PIN, INPUT_PULLUP);

  // Use EEPROM to store SETUP parameters.
  EEPROM_SIZE=sizeof(SETUP_STRUCT);
  
  if(EEPROM.begin(EEPROM_SIZE)){
    EEPROM.readBytes(EEPROM_START_ADDR,&tmp,EEPROM_SIZE);
    if(tmp.REVISION == SETUP.REVISION){ // Valid data exists and struct had not changed
      //Serial.println("reading from EEPROM");
      SETUP=tmp;
    }else{ // First time, so initalize
      //Serial.println("inital writing to EEPROM");
      EEPROM.writeBytes(EEPROM_START_ADDR,&SETUP,EEPROM_SIZE);
      EEPROM.commit();     
    }
    EEPROM_WORKING=1;  
  }
  STEPS=ENCODER_PULSE + SETUP.STEP_ADJUSTMENT;
  encoder.clearCount();

  Tft= new TFT_CLASS(SD_ENABLE,1); // Set  SD card=off/on  and initial rotation to 1
  //Tft->calibrate();

  // create touch key for main display
  DroKeyList = new KEYLIST_CLASS(Tft);
  DroKeyList->addArea(0,ROW[6]-1,Tft->w,ROW[6]+DRO_H+1,'S');

  // create touch keys for setup display
  SetupKeyList = new KEYLIST_CLASS(Tft);
  SetupKeyList->addButton(0,ROW[1]-1,80,ROW[1]+DRO_H+1,"Switch",'R');
  SetupKeyList->addButton(0,ROW[3]-1,80,ROW[3]+DRO_H+1,"-",'-');
  SetupKeyList->addButton(240,ROW[3]-1,319,ROW[3]+DRO_H+1,"+",'+');
  SetupKeyList->addButton(0,ROW[6]-1,159,ROW[6]+DRO_H+1,"Cancel",'C');
  SetupKeyList->addButton(159,ROW[6]-1,319,ROW[6]+DRO_H+1,"Apply",'A');
  
  // One rotation of encoder is ~40mm(1.57in). Want to reset buzzer after approx 12.7mm(0.5in) which is about 1 handle rotation.
  BUZZER_LIMIT=STEPS/3;  
  initDRODisplay(); 
}

void loop(){
  static unsigned long counter=0;

  if(millis()-counter >DRO_INTERVAL){  
    displayDRO(0);
    counter=millis();
  }
  checkSwitches();
  checkKeyboard();
  checkBuzzer();
}
