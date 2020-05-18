#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "tftDisplay.h"

class KEY_CLASS{
  private:
  public:
    TFT_CLASS *TftPtr;
    int xl,xr,yt,yb;
    int keyWidth,keyHeight;
    char uniqueID;

    KEY_CLASS();
    virtual char keyTouched(int x,int y,int textColor,int keyColor);
    virtual void keyDraw(int textColor,int backColor);
};

class AREA_CLASS: public KEY_CLASS{
  private:
  int type=0;
  public:
  AREA_CLASS(TFT_CLASS *,int x1,int y1,int x2,int y2,char value);
};

class BUTTON_CLASS: public KEY_CLASS{
  private:
  int type=1;
  char keyLabel[7];
  int scale=0,textXoffset=0,textYoffset=0;
  public:
  BUTTON_CLASS(TFT_CLASS *,int x1,int y1,int x2,int y2,const char *kl, char value);
  void keyDraw(int textColor,int backColor);
  char keyTouched(int x,int y,int textColor,int keyColor);
};

class RADIO_CLASS: public KEY_CLASS{
  private:
    int type=2;
    char *statePtr;
    char keyLabel[7];
    int scale=0,textXoffset=0,textYoffset=0;    
  public:
  RADIO_CLASS(TFT_CLASS *,int x1,int y1,int x2,int y2,const char *kl, char value,char *sPtr);
  void keyDraw(int textColor,int backColor);
  char keyTouched(int x,int y,int textColor,int keyColor);
  char getState();
};

class KEYLIST_CLASS{
  private:
    TFT_CLASS *TftPtr;
  public:
    KEY_CLASS *KeyList[40];
    int KeyCount=0;
    char stateData[100];
    
    KEYLIST_CLASS(TFT_CLASS *ptr);
    void drawKeys();
    char checkKeys();
    void addButton(int x1,int y1, int x2, int y2,const char *kl,char value);
    void addRadio(int x1,int y1, int x2, int y2,const char *kl,char value,int stateIndex);
    void addArea(int x1,int y1, int x2, int y2,char value);
    char getState(int index);
    void setState(int index,char value);
};

class KEYBOARD_CLASS{
  private:
        TFT_CLASS *TftPtr;
        int keyWidth;
        int keyHeight;
        KEYLIST_CLASS *KeyList[3];
        int CurrentScreen=0;
        int ScreenCount;
        int KeyCount[3]={0,0,0};
        int TextColor,KeyColor;
        int MaxEntryChars;
        
        void drawKeysOnly(String entryData);
        char checkKeys();
        String getEntry();
        void populateEntry(String myData);
  public:
    KEYBOARD_CLASS(TFT_CLASS *ptr,int rows,int cols, int fg, int bg, char *screenA,  char *screenB, char *screenC);
    String drawKeyboard(String label);
};

#endif
