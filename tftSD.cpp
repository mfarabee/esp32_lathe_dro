#include "tftSD.h"
#include "tftDisplay.h"
extern TFT_CLASS *Tft;

// Sample routines to access SD card

void SdVolume(){
    float mBytesPerCluster=(Tft->SdCard->vol()->blocksPerCluster()*512.0)/(1024.0*1024.0);
    
    Serial.print("Type: FAT");Serial.println(Tft->SdCard->vol()->fatType());
    Serial.printf("Total space: %4.2fMB\n", Tft->SdCard->vol()->clusterCount()*mBytesPerCluster);
    Serial.printf("Free space: %4.2fMB\n",  Tft->SdCard->vol()->freeClusterCount()*mBytesPerCluster);   
}

// List SD contents to serial terminal (top level routine)
void SdList(String dir){
  if(dir==""){
    dir="/";
  }
  Tft->SdCard->ls((char *)dir.c_str(),LS_SIZE|LS_R);
}

// No display:displayType=0,   console:displayType=1,   console&Tft:displayType=2
int SdFileExists(String fName, int displayType){
  int result=1;
    if(!Tft->SdCard->exists((char *)fName.c_str())){   
      result=0; 
    }
    if(displayType>0){
      Serial.print(fName);
      if(result==1){
        Serial.println(" exists!");
      }else{
        Serial.println(" does not exist!");
      }
      if(displayType==2){
        Tft->fillScreen(ILI9341_BLACK);
        Tft->setTextSize(2);   
        Tft->setCursor(0,0);
       Tft->print("SD: ");
        Tft->print(fName);
        if(result==1){
          Tft->println(" exists!");
        }else{
          Tft->println(" does not exist!");
        }
      }
    }
    
    return(result);
}


void SdReadFile(String fName){
  SdFat *sd;
  File dataFile;
  
  sd = Tft->getSdCard();
  if(sd->exists((char *)fName.c_str())){
    dataFile=sd->open(fName.c_str());
    if(dataFile.isFile()){
      while(dataFile.available()){
        Serial.write(dataFile.read());
      }
    }  
    dataFile.close();
  }else{
    Serial.print(fName);Serial.println(" not found");
  }
}

// List SD contents on TFT display (top level routine)
void SdListTft(String dir){
  SdFat *sd;
  File root;
  
  sd = Tft->getSdCard();
  Tft->setTextSize(2);
  if(dir==""){
    dir="/";
  }
  root=sd->open(dir.c_str());
  SdListTft(root,0);
  root.close();
}

void SdListTft(File dir,int spaces){
  uint8_t i;
  File entry;
  
  if(dir!=0){
    dir.rewindDirectory(); // This is needed to reset the dir pointer
    for (i=0;i<spaces;++i) { Tft->print(' '); }
    Tft->print(dir.name());
    if(dir.isDirectory()){
     if(strcmp(dir.name(),"/") !=0){
        Tft->println("/");
     }else{
      Tft->println();
     }
    }else{
      Tft->print("        ");
      Tft->println(dir.size(),DEC);
    }
    entry=dir.openNextFile();
    spaces=spaces+4;
    while(entry != 0){
      if(entry.isDirectory()){
       SdListTft(entry,spaces);
      }else{
        for (i=0;i<spaces;++i) { Tft->print(' '); }
          Tft->print(entry.name());
          Tft->print("\t\t");
          Tft->println(entry.size(),DEC);
        }
        entry.rewindDirectory(); // This is needed to reset the dir pointer for future access
        entry.close();
        entry=dir.openNextFile();
    }
  }
}
