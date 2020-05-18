#ifndef TFTSD_H
#define TFTSD_H
#include <Arduino.h>
#include <SdFat.h>  //  https://github.com/greiman/SdFat    


void SdVolume();
void SdList(String dir);
int SdFileExists(String fName, int displayType=0);
void SdReadFile(String fName);
void SdListTft(String dir);
void SdListTft(File dir,int spaces);

#endif
