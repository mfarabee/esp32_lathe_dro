#ifndef ESP32_LATHE_DRO_H
#define ESP32_LATHE_DRO_H


//DRO_INTERVAL is how often the display is updated 
#define DRO_INTERVAL 100

// This is for the local SD card on the TFT. Not currently being used by this program
// Enable of SD card, enable=1, disable=0
#define SD_ENABLE 0

// ENCODER TYPE F=FULL(2400), H=HALF(1200), S=SINGLE(600)
#define ENCODER_TYPE  'F'
int ENCODER_PULSE=0;

#define A_PIN 16  //GREEN
#define B_PIN 17  // WHITE

#define INC_PIN 21
#define ZERO_PIN 22
#define BUZZER_IN_PIN 2
#define BUZZER_OUT_PIN 32

#define ABSOLUTE 1
#define INCREMENTAL -1

#define DEBOUNCE_TIME 500

// DRO DISPLAY LAYOUT
#define DRO_SCALE 4
#define DRO_H 32
#define DRO_H_HALF 16

#define DRO_W 24
#define DRO_DIGITS 10
#define DRO_STARTX 48


#define EEPROM_START_ADDR 0
int EEPROM_SIZE=0;
int EEPROM_WORKING =0;

int ROW[]={1,35,69,103,137,171,205};

//int SWAPX = 1;
int STEPS = 0;
//int STEP_ADJUSTMENT = 0;

// change revision everytime struct is modified to force reload of eeprom
typedef struct {
    int REVISION=1;
    int STEP_ADJUSTMENT=0;
    int SWAPX=1;
} SETUP_STRUCT;

SETUP_STRUCT SETUP;

#define BUZZER_TIME 500
#define BUZZER_OFF 0
#define BUZZER_READY 1
#define BUZZER_WAIT_FOR_RESET 2
int64_t BUZZER_LIMIT= 800;
int64_t BUZZER_COUNT=0;
int     BUZZER_STATE=0;

void displayDRO(int resetDisplay=0);


#endif
