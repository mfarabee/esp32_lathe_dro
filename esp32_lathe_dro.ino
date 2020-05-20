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
int32_t counterOffset=0;
KEYLIST_CLASS *DroKeyList,*SetupKeyList;
int Absolute=1;

void initDRODisplay(){
  Tft->fillScreen(ILI9341_BLACK);
  DroKeyList->drawKeys();
  AbsInc();
  Tft->fillRect(0,ROW[6], Tft->w, DRO_H,ILI9341_BLUE);
  Tft->setTextColor(ILI9341_WHITE); Tft->setCursor(80,ROW[6]);
  Tft->print("Setup");
  if(Absolute >0){
      displayDRO(encoder.getCountRaw(),1);
  }else{
      displayDRO(encoder.getCount(),1);
  }
}

void displaySetup(){
  int32_t stepCnt;
  int swap;
  char value;
  
  swap=SWAPX;
  stepCnt=STEPS;
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
        case 'R':
          swap *= -1;
          Tft->fillRect(100,ROW[1],Tft->w-100,DRO_H,ILI9341_BLACK);
          Tft->setCursor(100,ROW[1]); Tft->print(swap);
        break;
        case '-':
        stepCnt -= 1;
        Tft->fillRect(100,ROW[3],140,DRO_H,ILI9341_BLACK);
        Tft->setCursor(100,ROW[3]); Tft->print(stepCnt);
        break;
        case '+':
        stepCnt += 1;
        Tft->fillRect(100,ROW[3],140,DRO_H,ILI9341_BLACK);
        Tft->setCursor(100,ROW[3]); Tft->print(stepCnt);
        break;        
        }
    }
    value=SetupKeyList->checkKeys();
  }
  if(value == 'A'){
    SWAPX=swap;
    STEPS=stepCnt;
    if(EEPROM_WORKING == 1){
      EEPROM.writeInt(EEPROM_DIR_ADDR,SWAPX);
      EEPROM.writeInt(EEPROM_SCALE_ADDR,STEPS);
      Serial.println("writing to EEPROM");
    }
  }
}

// Displays Coordinates
void displayDRO(int32_t countX,int resetDisplay){
  char tmp[100];
  float cXin,cXmm;
  static char prevXmm[20]="",prevXin[20]="";
  /*
   *         ABS/INC
   * X     1234.1234
   * Y     1234.1234
   */

   cXmm=SWAPX*40.0*countX/(float)STEPS;

   cXin=cXmm/25.4;
   
  Tft->setTextSize(DRO_SCALE);
  if(resetDisplay==1){
     strcpy(prevXmm,"");strcpy(prevXin,"");
     Tft->fillRect(DRO_STARTX,ROW[2], DRO_DIGITS*DRO_W, DRO_H,ILI9341_BLACK);
     Tft->fillRect(DRO_STARTX,ROW[3],DRO_DIGITS*DRO_W, DRO_H,ILI9341_BLACK);
  }


  sprintf(tmp,"%-5.4f mm",(float)cXmm); 
  if(strcmp(tmp,prevXmm)!=0){ // Only print if different
    Tft->setTextColor(ILI9341_BLACK); Tft->setCursor(DRO_STARTX+(DRO_W*(DRO_DIGITS-strlen(prevXmm))),ROW[2]); Tft->print(prevXmm);
    Tft->setTextColor(ILI9341_YELLOW); Tft->setCursor(DRO_STARTX+(DRO_W*(DRO_DIGITS-strlen(tmp))),ROW[2]); Tft->print(tmp);
    strcpy(prevXmm,tmp);
        
    sprintf(tmp,"%-5.4f in",(float)cXin); 
    Tft->setTextColor(ILI9341_BLACK); Tft->setCursor(DRO_STARTX+(DRO_W*(DRO_DIGITS-strlen(prevXin))),ROW[4]); Tft->print(prevXin);
    Tft->setTextColor(ILI9341_YELLOW); Tft->setCursor(DRO_STARTX+(DRO_W*(DRO_DIGITS-strlen(tmp))),ROW[4]); Tft->print(tmp);
    strcpy(prevXin,tmp);
  }
}

int debounce(int pinNum){
  static unsigned long debounce[50]={0L};
  unsigned long debounceTime=0;
  int result=1;
  
  if(debounce[pinNum] == 0L){
    debounce[pinNum]=millis();
    Serial.println("set");         
  }else {
    debounceTime=(millis()- debounce[pinNum]);
    if(debounceTime < DEBOUNCE_TIME){
      Serial.print(debounceTime);Serial.print("   ");Serial.println("fail"); 
      result=0;
    }else{
      debounce[pinNum]=millis();
      Serial.println("set2"); 
    }
  }
  Serial.print(pinNum);Serial.print("  ");Serial.println(debounce[pinNum]);
  return(result);
}

void checkSwitches(){
  int32_t tmp,raw;
  if(digitalRead(ZERO_PIN)==LOW){
    if(debounce(ZERO_PIN)==1){
      if(Absolute>0){
        tmp=encoder.getCount();
        raw=encoder.getCountRaw();
        encoder.clearCount();
        encoder.setCount(tmp-raw);
        drawBuzzer(0);
      }else{
        encoder.setCount(0);
      }    
    }
  }
  if(digitalRead(INC_PIN)==LOW){
    if(debounce(INC_PIN)==1){
        Absolute *= -1;
        Serial.println(Absolute);
        AbsInc();
    }
  }
  if(digitalRead(BUZZER_IN_PIN)==LOW){
    if(debounce(BUZZER_IN_PIN)==1){
      if(BUZZER_ACTIVE>0){
        drawBuzzer(0);
      }else{
        drawBuzzer(2);
        BUZZER_COUNT=encoder.getCountRaw();
        BUZZER_PREV=BUZZER_COUNT;
      }
      drawBuzzer(BUZZER_ACTIVE);
    }
  }
} 

void AbsInc(){
  Tft->setTextSize(DRO_SCALE);
  Tft->fillRect(0,ROW[0], Tft->width(), DRO_H,ILI9341_BLACK);
  Tft->setTextColor(ILI9341_WHITE); Tft->setCursor(40,ROW[0]);
  if(Absolute>0){
     Tft->print(" ABSOLUTE");
  }else{
    Tft->print("INCREMENTAL");
  } 
}

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

void drawBuzzer(int flag){
  BUZZER_ACTIVE=flag;
  Tft->setTextSize(DRO_SCALE);
  Tft->fillRect(0,ROW[5], Tft->w, DRO_H,ILI9341_BLACK);
  switch(flag){
    case 1:
      Tft->setTextColor(ILI9341_GREEN);
      Tft->setCursor(0,ROW[5]);Tft->print("BUZZER");
    break;
    case 2:
      Tft->setTextColor(ILI9341_RED);
      Tft->setCursor(0,ROW[5]);Tft->print("BUZZER");
    break;
    default:
    break;
  }    
}

void checkBuzzer(){
  int32_t currentCnt;

  if(BUZZER_ACTIVE != 0){
    currentCnt=encoder.getCountRaw();

    // if current has moved outside limit, enable buzzer(1)
    if(BUZZER_ACTIVE==2 && abs(currentCnt-BUZZER_COUNT)>BUZZER_LIMIT){
      drawBuzzer(1);
    }
  
    // if current hits BUZZER_COUNT 
    if(BUZZER_ACTIVE==1){
      if((currentCnt < BUZZER_PREV && currentCnt<=BUZZER_COUNT) ||  // moving left
         (currentCnt > BUZZER_PREV && currentCnt>=BUZZER_COUNT)){   // moving right
        // buzz for 1 second
        drawBuzzer(2);
      }
    }   
    BUZZER_PREV=currentCnt;
  }
}


void setup() {
  String result="";

  Serial.begin(115200);
  ESP32Encoder::useInternalWeakPullResistors=false;
  
  encoder.attachHalfQuad(A_PIN, B_PIN); //1200
  //encoder.attachFullQuad(A_PIN, B_PIN); //2400

  pinMode(A_PIN, INPUT_PULLUP);
  pinMode(B_PIN, INPUT_PULLUP);
  
  EEPROM_DIR_ADDR=EEPROM_VALID_ADDR+sizeof(char);
  EEPROM_SCALE_ADDR=EEPROM_DIR_ADDR+sizeof(int);
  EEPROM_SIZE= sizeof(char)+sizeof(int)+sizeof(int);
  if(EEPROM.begin(EEPROM_SIZE)){
    if(EEPROM.readChar(EEPROM_VALID_ADDR) == 'X'){ //Valid data exists
      SWAPX=EEPROM.readInt(EEPROM_DIR_ADDR);
      STEPS=EEPROM.readInt(EEPROM_SCALE_ADDR);
    }else{ // First time, so initalize
      Serial.println("inital writing to EEPROM");
      EEPROM.writeChar(EEPROM_VALID_ADDR,'X');
      EEPROM.writeInt(EEPROM_DIR_ADDR,SWAPX);
      EEPROM.writeInt(EEPROM_SCALE_ADDR,STEPS);
      
    }
    EEPROM_WORKING=1;  
  }
  
  encoder.clearCount();
   
  pinMode(ZERO_PIN, INPUT_PULLUP);
  pinMode(INC_PIN, INPUT_PULLUP);
  pinMode(BUZZER_IN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_OUT_PIN, OUTPUT);
 
  Tft= new TFT_CLASS(SD_ENABLE,1); // Set  SD card=off/on  and initial rotation to 1
  //Tft->calibrate();
  
  DroKeyList = new KEYLIST_CLASS(Tft);
  //DroKeyList->addArea(0,ROW[0]-1,Tft->w,ROW[0]+DRO_H+1,'A');
  DroKeyList->addArea(0,ROW[6]-1,Tft->w,ROW[6]+DRO_H+1,'S');

  SetupKeyList = new KEYLIST_CLASS(Tft);
  SetupKeyList->addButton(0,ROW[1]-1,80,ROW[1]+DRO_H+1,"Switch",'R');
  SetupKeyList->addButton(0,ROW[3]-1,80,ROW[3]+DRO_H+1,"-",'-');
  SetupKeyList->addButton(240,ROW[3]-1,319,ROW[3]+DRO_H+1,"+",'+');
  SetupKeyList->addButton(0,ROW[6]-1,159,ROW[6]+DRO_H+1,"Cancel",'C');
  SetupKeyList->addButton(159,ROW[6]-1,319,ROW[6]+DRO_H+1,"Apply",'A');
  AbsInc();

  initDRODisplay(); 
}

void loop(){

  static unsigned long counter=0;

  if(millis()-counter >DRO_INTERVAL){
    if(Absolute >0){
      displayDRO(encoder.getCountRaw(),0);
    }else{
      displayDRO(encoder.getCount(),0);
    }
    counter=millis();
  }
  checkSwitches();
  checkKeyboard();
  checkBuzzer();
}
