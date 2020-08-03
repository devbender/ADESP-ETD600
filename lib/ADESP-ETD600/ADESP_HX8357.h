#pragma once

//#include <JPEGDecoder.h>
#include <TJpg_Decoder.h>
#include <Adafruit_HX8357.h>
#include "FS.h"
#include "SPIFFS.h"

// LCD BACKLIGHT CONTROL -----------------------------------------------
#define PWM_CHAN	0
#define PWM_RES		8 		// 8bits
#define PWM_FREQ	5000	// 5Khz

#define LCD_OFF 	0
#define LCD_DIMMED 	20
#define LCD_ON 		255


//#######################################################################
// *CLASS* >>> TFT DRIVER SUBCLASS DEFINITION
//#######################################################################
class ADESP_HX8357: public Adafruit_HX8357 {

private:
	uint8_t PWM_PIN;
	uint8_t BRIGHTNESS = 0;

	void jpegRender(int xpos, int ypos);
	bool tft_output2(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

public:
	ADESP_HX8357(int CS, int DC, int RST) : Adafruit_HX8357(CS, DC, RST) {}
	ADESP_HX8357(int CS, int DC, int RST, int PWM);	
	
	void eraseText(int, uint16_t, uint16_t, uint16_t);
	void eraseText(const char*, uint16_t, uint16_t, uint16_t);
	
    void printCentered(const char*, uint16_t, uint16_t);
	void printCenteredAbs(const char*);
    void printCenteredAbs(const char*, int16_t, int16_t);	

	void drawRhomb(int16_t, int16_t,int16_t, uint16_t);	

	void enablePWM();
	void enablePWM(uint8_t pwmPin);
	void setBRT(uint8_t brt);
	uint8_t getBRT(void);
	
	void fadeIn();
	void fadeOut();
	void fadeInOut(uint16_t delayMS);

	void drawJpeg(fs::File file, int xpos, int ypos);
	void drawFSJpeg(fs::FS &fs, const char *filename, int xpos, int ypos);

	


};