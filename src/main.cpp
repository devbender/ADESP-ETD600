#include <ADESP_ETD600.h>

// ESP32 PIN DEFINES
#define SD_CS		14
#define TFT_CS		15
#define TFT_DC		33
#define TFT_RST		-1
#define TFT_PWM		21
#define STMPE_CS	32
#define STMPE_INT	27

ADESP_STMPE610 ts(STMPE_CS, STMPE_INT, onTS_TOUCH);
ADESP_HX8357 tft(TFT_CS, TFT_DC, TFT_RST, TFT_PWM);

//########################################################################
// SETUP
//########################################################################
void setup() {    
  ADESP_SYS_INIT();
  ADESP_SYS_START();

  
}

//########################################################################
// MAIN LOOP
//########################################################################
void loop() {

  if(Serial.available()) {
    char cmd = Serial.read();

    switch (cmd) {
      case 'D': {
        tft.setBRT(0);
        Serial.println("CMD | MIN BRIGHT");
        break;
      }
    
      case 'U': {
        Serial.println("CMD | MAX BRIGHT");
        tft.setBRT(255);
        break;
      }

      default: break;
    }
  }
}
  