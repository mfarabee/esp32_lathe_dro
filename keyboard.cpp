#include "keyboard.h"

/*  Keyboard: These classes (KEY_CLASS ,KEYLIST_CLASS and KEYBOARD_CLASS) allows the programmer to create a keyboard for data entry.
 *   Two types of keyboards can be generated. 
 *   1) By creating a KEYLIST_CLASS, the programmer can create keys at specific locations.
 *   2) KEYBOARD_CLASS will automatically create a keyboard based on a list of characters.
 * A keyboard can consist of up to 3 screens containing 40 user defined keys per screen. Each screen will be populated
 * with five additional keys (Cancel,Enter,Backspace,Shift,Clear) as well as an entry box to display what is typed.
 * The programmer can control the key color, text color as will as the number of rows and columns. It is up to the user
 * To make sure that the row/column entries will suppoert the number of keys defined (row * col >= kumber of keys).
 * Once the keyboard is defined, a call to drawKeyboard will display the keyboard and return a string containing the
 * resulting input.
 * 
 * When creating a KEYLIST, individual keys are created using the addButton, addArea, addRadio methods. 
 * All return values for a KEYLIST or KEYBOARD must be unique.
 * addButton - Created a button with a label. the value is returned when pressed.
 * addArea   - This creates a touchable area (button) without a label.
 * addRadio  - This creates a radio button/buttons. Only one button in the same group is active at any one time.
 * 
 * 
 * 
 * For both KEYLIST and KEYBOARD, the checkKeys method will check if the screen is pressed for all keys defined in that screen or list.
*/


#define KEY_DELAY 400 // delay to slow rapid entry if key is held down (repeat character speed)
#define EntrySize 40  // height of entry box in pixels
#define EntryScale 3  // text scale for entery box
#define KbdBackgroundColor  ILI9341_BLACK
#define EMPTY_CHAR 0x0
#define CANCEL_CHAR 0x01
#define BACKSPACE_CHAR 0x08
#define ENTER_CHAR 0X0D
#define SHIFT_CHAR 0x18
#define CLEAR_CHAR 0xAE

typedef struct {
  char value;
  char label[7];
} KbdStruct;
const KbdStruct LastRow[] ={{SHIFT_CHAR,{SHIFT_CHAR,'\0'}},{CANCEL_CHAR,"Cncl"},{BACKSPACE_CHAR,"Bksp"},{ENTER_CHAR,"Enter"}};

/****************************
      KEY CLASS METHODS
****************************/
// KEY_CLASS
//Constructor for Key Class
KEY_CLASS::KEY_CLASS(){
  //TftPtr=ptr;
  //keyWidth=x2-x1; keyHeight=y2-y1; uniqueID=value;
  //xl=x1; xr=x2; yt=y1; yb=y2;
  //if(strcmp(kl,"") !=0){
    //strncpy(keyLabel,kl,6);
    //keyLabel[6]= '\0'; // Put end of string char, since strncpy will not do this
    // Determine scale factor for text based on size of key
    //scale=min(((keyWidth-2)/((int)strlen(keyLabel)*6)),(keyHeight-2)/8);
    // Calculate text location from edge of box (center text)
    //textXoffset=(keyWidth-((strlen(keyLabel)*scale*6)-1))/2;
    //textYoffset=(keyHeight-((scale*8)-1))/2;
  //}
}

// KEYDRAW
// Draw single key based on stored information
void KEY_CLASS::keyDraw(int textColor,int backColor){
}

// KEY TOUCHED
// Check if touch point is within the boundary of key location.
// Will "flash" key by reversing text and key colors when key is pressed
char  KEY_CLASS::keyTouched(int x,int y, int textColor, int keyColor){
  char result='\0';
  if(x>=xl && x<=xr && y>=yt && y<=yb){
    result=uniqueID;
    /*
    if(scale !=0){
      keyDraw(keyColor,textColor);
      delay(100);
      keyDraw(textColor,keyColor);
    }
    */
    delay(KEY_DELAY);  // delay to slow rapid entry if key is held down (repeat character speed)
  }
  return(result);
}


// BUTTON_CLASS
//Constructor for BUTTON Class
BUTTON_CLASS::BUTTON_CLASS(TFT_CLASS *ptr,int x1,int y1, int x2, int y2,const char *kl,char value){
  TftPtr=ptr;
  keyWidth=x2-x1; keyHeight=y2-y1; uniqueID=value;
  xl=x1; xr=x2; yt=y1; yb=y2;scale=0;
  if(strcmp(kl,"") !=0){
    strncpy(keyLabel,kl,6);
    keyLabel[6]= '\0'; // Put end of string char, since strncpy will not do this
    // Determine scale factor for text based on size of key
    scale=min(((keyWidth-2)/((int)strlen(keyLabel)*6)),(keyHeight-2)/8);
    // Calculate text location from edge of box (center text)
    textXoffset=(keyWidth-((strlen(keyLabel)*scale*6)-1))/2;
    textYoffset=(keyHeight-((scale*8)-1))/2;
  }
}
char  BUTTON_CLASS::keyTouched(int x,int y, int textColor, int keyColor){
  char result='\0';
  if(x>=xl && x<=xr && y>=yt && y<=yb){
    result=uniqueID;
    if(scale !=0){
      keyDraw(keyColor,textColor);
      delay(100);
      keyDraw(textColor,keyColor);
    }
    delay(KEY_DELAY);  // delay to slow rapid entry if key is held down (repeat character speed)
  }
  return(result);
}
void BUTTON_CLASS::keyDraw(int textColor,int backColor){
  if(scale !=0){
    TftPtr->fillRoundRect(xl,yt, keyWidth-2, keyHeight-2,5,backColor);
    TftPtr->setTextColor(textColor);
    TftPtr->setTextSize(scale);
    TftPtr->setCursor(xl+textXoffset,yt+textYoffset);
    TftPtr->print(keyLabel);
  }
}

AREA_CLASS::AREA_CLASS(TFT_CLASS *ptr,int x1,int y1, int x2, int y2,char value){
  TftPtr=ptr;
  keyWidth=x2-x1; keyHeight=y2-y1; uniqueID=value;
  xl=x1; xr=x2; yt=y1; yb=y2;
}

// RADIO_CLASS
//Constructor for RADIO Class
RADIO_CLASS::RADIO_CLASS(TFT_CLASS *ptr,int x1,int y1, int x2, int y2,const char *kl,char value,char *sPtr){
  TftPtr=ptr;
  keyWidth=x2-x1; keyHeight=y2-y1; uniqueID=value; statePtr=sPtr;
  *statePtr='\0';
  xl=x1; xr=x2; yt=y1; yb=y2;scale=0;
  if(strcmp(kl,"") !=0){
    strncpy(keyLabel,kl,6);
    keyLabel[6]= '\0'; // Put end of string char, since strncpy will not do this
    // Determine scale factor for text based on size of key
    scale=min(((keyWidth-2)/((int)strlen(keyLabel)*6)),(keyHeight-2)/8);
    // Calculate text location from edge of box (center text)
    textXoffset=(keyWidth-((strlen(keyLabel)*scale*6)-1))/2;
    textYoffset=(keyHeight-((scale*8)-1))/2;
  }
}

void RADIO_CLASS::keyDraw(int textColor,int backColor){
  if(scale !=0){
    if(*statePtr != uniqueID){
      TftPtr->fillRoundRect(xl,yt, keyWidth-2, keyHeight-2,5,backColor);
      TftPtr->setTextColor(textColor);
    }else{
      TftPtr->fillRoundRect(xl,yt, keyWidth-2, keyHeight-2,5,textColor);
      TftPtr->setTextColor(backColor);
    }
    TftPtr->setTextSize(scale);
    TftPtr->setCursor(xl+textXoffset,yt+textYoffset);
    TftPtr->print(keyLabel);
  }
}
char RADIO_CLASS::getState(){
  return(*statePtr);
}
char  RADIO_CLASS::keyTouched(int x,int y, int textColor, int keyColor){
  char result='\0';
  if(x>=xl && x<=xr && y>=yt && y<=yb){
    result=uniqueID;
    if(*statePtr==uniqueID){ //
      *statePtr='\0';
    }else{
      *statePtr=uniqueID;
    }
    keyDraw(textColor,keyColor);

    delay(KEY_DELAY);  // delay to slow rapid entry if key is held down (repeat character speed)
  }
  return(result);
}
/***************************************
 *  KEYLIST CLASS METHODS
***************************************/

KEYLIST_CLASS::KEYLIST_CLASS(TFT_CLASS *ptr){
  TftPtr=ptr;
}

void KEYLIST_CLASS::drawKeys(){
  int TextColor=ILI9341_WHITE,KeyColor=ILI9341_BLUE;
  int i;
    for(i=0;i<KeyCount;++i){
    KeyList[i]->keyDraw(TextColor,KeyColor);
  }
}

char KEYLIST_CLASS::checkKeys(){
  int i;
  char result=0;
  int TextColor=ILI9341_WHITE,KeyColor=ILI9341_BLUE;
  int x, y;
  
  if(TftPtr->isTouched(&x, &y)) {
    for(i=0;i<KeyCount;++i){
      result=KeyList[i]->keyTouched(x,y,TextColor,KeyColor);
      if(result != 0){
        //Serial.print("  checkKeys: ");Serial.println(result);
        break;
      }
    }
  }
  return(result);
}

void KEYLIST_CLASS::addButton(int x1,int y1, int x2, int y2,const char *kl,char value){
  if(KeyCount <40){
    KeyList[KeyCount]=new BUTTON_CLASS(TftPtr,x1,y1,x2,y2,kl,value);
    ++KeyCount;
  }
}
void KEYLIST_CLASS::addArea(int x1,int y1, int x2, int y2,char value){
  if(KeyCount <40){
    KeyList[KeyCount]=new AREA_CLASS(TftPtr,x1,y1,x2,y2,value);
    ++KeyCount;
  }
}
void KEYLIST_CLASS::addRadio(int x1,int y1, int x2, int y2,const char *kl,char value,int stateIndex){
  if(KeyCount <40){
    KeyList[KeyCount]=new RADIO_CLASS(TftPtr,x1,y1,x2,y2,kl,value,&(stateData[stateIndex]));
    ++KeyCount;
  }
}
void KEYLIST_CLASS::setState(int index,char value){
  stateData[index]=value;
}
char KEYLIST_CLASS::getState(int index){
return(stateData[index]);
}
/*****************************
   KEYBOARD CLASS METHODS
*****************************/

// KEYBOARD CLASS
// constructor for KEYBOARD Class
KEYBOARD_CLASS::KEYBOARD_CLASS(TFT_CLASS *ptr,int rows,int cols,int fg,int bg,  char *screenA,  char *screenB, char *screenC){
  int col,row,i,cnt,lastWidth,lastCnt,screen;
  char label[2];
  char *screens[3];
  
  TftPtr=ptr;
  TextColor=fg; KeyColor=bg;
  MaxEntryChars=(TftPtr->width()-EntrySize)/(EntryScale*6);

  // Determine key count for each of the three possible screens
  ScreenCount=0;
  if(strlen(screenA)>0){
  screens[ScreenCount]=screenA;
  KeyCount[ScreenCount]=strlen(screenA);
  ++ScreenCount;
  }
  if(strlen(screenB)>0){
  screens[ScreenCount]=screenB;
  KeyCount[ScreenCount]=strlen(screenB);
  ++ScreenCount;
  }
  if(strlen(screenC)>0){
  screens[ScreenCount]=screenC;
  KeyCount[ScreenCount]=strlen(screenC);
  ++ScreenCount;
  }

  // Calculate key height based on row and columns
  keyWidth=TftPtr->width()/cols;  
  keyHeight=(TftPtr->height()-EntrySize)/(rows+1);  

  // Loop through screen keys and create key instances at the proper location
  for(screen=0;screen<ScreenCount;++screen){  
    col=0;row=0;
    KeyList[screen]=new KEYLIST_CLASS(ptr);
    for(i=0;i<KeyCount[screen];++i){
      label[0]=screens[screen][i];label[1]='\0';
      KeyList[screen]->addButton((col*keyWidth),(row*keyHeight)+EntrySize,(col*keyWidth)+keyWidth,(row*keyHeight)+EntrySize+keyHeight,label,screens[screen][i]);
      ++col;
      if(col>=cols){
        col=0;
        ++row;
      }
    }

    // Add the CLEAR key on the entry line
    label[0]=CLEAR_CHAR;label[1]='\0';
    //KeyList[screen][KeyCount[screen]]=new BUTTON_CLASS(TftPtr,(TftPtr->width()-EntrySize,0,TftPtr->width(),EntrySize,label,CLEAR_CHAR);
    KeyList[screen]->addButton(TftPtr->width()-EntrySize,0,TftPtr->width(),EntrySize,label,CLEAR_CHAR);
    ++KeyCount[screen];
    
    cnt=0;
    i=KeyCount[screen];

    // Add the last row
    lastCnt=sizeof(LastRow)/sizeof(KbdStruct);
    KeyCount[screen] +=lastCnt;
    if(ScreenCount <=1){ // if there is only one screen, do not include shift key (first key in struct)
      --KeyCount[screen];
      ++cnt;
      lastWidth=TftPtr->width()/(lastCnt-1);
    }else{
       lastWidth=TftPtr->width()/lastCnt;
    }
    col=0;
    row=(rows*keyHeight)+EntrySize;
    while(cnt<lastCnt){
      //KeyList[screen][i]=new KEY_CLASS(TftPtr,(col*lastWidth),row,(col*lastWidth)+lastWidth,row+keyHeight,LastRow[cnt].label,LastRow[cnt].value);
      KeyList[screen]->addButton((col*lastWidth),row,(col*lastWidth)+lastWidth,row+keyHeight,LastRow[cnt].label,LastRow[cnt].value);
        ++col;
        ++i;
        ++cnt;
      }    
  }
}

void KEYBOARD_CLASS::drawKeysOnly(String entryData){
  TftPtr->fillScreen(KbdBackgroundColor);
  KeyList[CurrentScreen]->drawKeys();
  populateEntry(entryData);
}


// ACTIVATE KEYBOARD
// public function that draws keyboard and returns result
String KEYBOARD_CLASS::drawKeyboard(String label){
  drawKeysOnly(label);
  return(getEntry());
}


// CHECK KEYS
// Checks if key is pressed by checking X/Y against all keys on current screen
char KEYBOARD_CLASS::checkKeys(){
  return(KeyList[CurrentScreen]->checkKeys());
}

// POPULATE ENTRY
//prints entry data in entry line. If the line is too short, it will only display that ending subset of charaters
void KEYBOARD_CLASS::populateEntry(String myData){
  int startChar;
  TftPtr->fillRect(0,0, TftPtr->width()-50, EntrySize,KbdBackgroundColor);
  TftPtr->setTextSize(EntryScale);   
  TftPtr->setCursor(0,0);
  if(myData.length() <= MaxEntryChars){
    startChar=0;
  }else{
    startChar=myData.length()- MaxEntryChars;
  }
  TftPtr->print(myData.substring(startChar));
}

// GET ENTRY
// If key is pressed, determine if it is data or a special charater that needs special handling (cancel, clear, enter, etc...)
String KEYBOARD_CLASS::getEntry(){
  int foundEnter=0,pos=0;
  String myData="";
  char result;
  while(foundEnter==0){
    result=checkKeys();
    if(result != '\0'){  
      if(result== ENTER_CHAR){             // Enter
        foundEnter=1;
      }else if(result == BACKSPACE_CHAR){  // Backspace
            if(pos>0){
              myData.remove(myData.length()-1,1);
              --pos;
              populateEntry(myData);
            }
      }else if(result == CANCEL_CHAR){     // Cancel
        myData="";
        foundEnter=1;
      }else if(result == SHIFT_CHAR){      // Shift (new screen)
        ++CurrentScreen;
        if(CurrentScreen >= ScreenCount){
          CurrentScreen=0;
        }
        drawKeysOnly(myData);
      }else if(result == CLEAR_CHAR){      // Clear
        TftPtr->fillRect(0,0, TftPtr->width()-50, EntrySize,KbdBackgroundColor);
        pos=0;
        myData="";
      }else if(result != 0){              // Data charater
        myData.concat(result);
        populateEntry(myData);
        ++pos;
      }
   }
  }
   TftPtr->fillScreen(KbdBackgroundColor);
   //Serial.print("result= ");Serial.println(myData);
   return(myData);
}
