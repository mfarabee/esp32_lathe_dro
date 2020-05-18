#ifndef ESP32_LATHE_DRO_H
#define ESP32_LATHE_DRO_H


//DRO_INTERVAL is how often the display is updated (send '?' to controller)
//It is recommended that you do not send a command faster than about 200ms (5 hz)
#define DRO_INTERVAL 200

// This is for the local SD card on the TFT. Not currently being used by this program
// Enable of SD card, enable=1, disable=0
#define SD_ENABLE 0

#define A_PIN 16  //GREEN
#define B_PIN 17  // WHITE
#define INC_PIN 21
#define ZERO_PIN 22
#define BUZZER_PIN 1

#define DEBOUNCE_TIME 500

// DRO DISPLAY LAYOUT
#define DRO_SCALE 4
#define DRO_H 32
#define DRO_H_HALF 16

#define DRO_W 24
#define DRO_DIGITS 10
#define DRO_STARTX 48

#define EEPROM_SIZE 3
int EEPROM_WORKING =0;

int ROW[]={1,35,69,103,137,171,205};
int SWAPX = 1;
int SWAPY= 1;
int STEPS = 1200;
int BUZZER_COUNT=0;
int BUZZER_ACTIVE=0;


void displayDRO(int resetDisplay=0);

#endif
