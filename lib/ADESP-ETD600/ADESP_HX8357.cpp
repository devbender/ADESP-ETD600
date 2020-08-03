#include "ADESP_HX8357.h"


// Return the minimum of two values a and b
#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

extern ADESP_HX8357 tft;

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // In ILI9341 library this function clips the image block at TFT boundaries
  tft.drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}

//#################################################################################################################
// *CLASS* >>> TFT DRIVER SUBCLASS
//#################################################################################################################
ADESP_HX8357::ADESP_HX8357(int CS, int DC, int RST, int PWM)
	: Adafruit_HX8357(CS, DC, RST), PWM_PIN(PWM) {

	ledcSetup(PWM_CHAN, PWM_FREQ, PWM_RES);
	ledcAttachPin(PWM_PIN, PWM_CHAN);
	ledcWrite(PWM_CHAN, BRIGHTNESS);

	TJpgDec.setJpgScale(1);
	TJpgDec.setCallback(tft_output);
}

void ADESP_HX8357::enablePWM(uint8_t pwmPin) {	

	ledcSetup(PWM_CHAN, PWM_FREQ, PWM_RES);
	ledcAttachPin(pwmPin, PWM_CHAN);
	ledcWrite(PWM_CHAN, BRIGHTNESS);
}

void ADESP_HX8357::setBRT(uint8_t level) {
	BRIGHTNESS = level;
	ledcWrite(PWM_CHAN, level);
}

uint8_t ADESP_HX8357::getBRT(void) {
	return BRIGHTNESS;
}

void ADESP_HX8357::fadeOut() {
	
	int rate = 1;
	
	for(int i=BRIGHTNESS; i>=0; i--){ 
		ledcWrite(PWM_CHAN, i);
		vTaskDelay(rate / portTICK_PERIOD_MS);
	}	
}

void ADESP_HX8357::fadeIn() {
	
	int rate = 1;
	
	if(BRIGHTNESS == 0) BRIGHTNESS = 255;

	for(int i=0; i<=BRIGHTNESS; i++) {
		ledcWrite(PWM_CHAN, i);
		vTaskDelay(rate / portTICK_PERIOD_MS);
	}
}

void ADESP_HX8357::fadeInOut(uint16_t delayMS) {
	fadeIn();
	delay(delayMS);
	fadeOut();

	BRIGHTNESS = 0;
}


void ADESP_HX8357::eraseText(const char* str, uint16_t xPos, uint16_t yPos, uint16_t color) {
	int16_t x,y;
	uint16_t w,h;
	
	getTextBounds(str, xPos, yPos, &x, &y, &w, &h);
	fillRect(xPos, yPos, w, h, color);
}

void ADESP_HX8357::eraseText(int n, uint16_t xPos, uint16_t yPos, uint16_t color) {
	int16_t x,y;
	uint16_t w,h;

	char str[8];
	sprintf(str, "%d", n);	
	getTextBounds(str, xPos, yPos, &x, &y, &w, &h);
	fillRect(xPos, yPos, w, h, color);
}

void ADESP_HX8357::printCentered(const char* str, uint16_t X, uint16_t Y) {

	int16_t x,y;
	uint16_t w,h;
    uint16_t Xc, Yc;
    
	getTextBounds(str, 0, 0, &x, &y, &w, &h);

	Xc = X - (w / 2);
	Yc = Y - (h /2);

	setCursor(Xc, Yc);
	print(str);
}


void ADESP_HX8357::printCenteredAbs(const char* str) {

	int16_t x,y;
	uint16_t w,h;
    uint16_t Xc, Yc;
    
	getTextBounds(str, 0, 0, &x, &y, &w, &h);

	Xc = (width() - w) / 2;
	Yc = (height() - h) /2;

	setCursor(Xc, Yc); 
	print(str);
}

void ADESP_HX8357::printCenteredAbs(const char* str, int16_t Xo, int16_t Yo) {

	int16_t x,y;
	uint16_t w,h;
    uint16_t Xc, Yc;
    
	getTextBounds(str, 0, 0, &x, &y, &w, &h);

	Xc = (width() - w) / 2;
	Yc = (height() - h) /2;

	setCursor(Xc + Xo, Yc + Yo); 
	print(str);
}


void ADESP_HX8357::drawRhomb(int16_t x0, int16_t y0, int16_t size, uint16_t color) {
	drawLine(x0, 		y0 - size, 	x0 - size, 	y0, 		color);
	drawLine(x0, 		y0 - size, 	x0 + size, 	y0, 		color);
	drawLine(x0 - size, y0, 		x0, 		y0 + size, 	color);
	drawLine(x0 + size, y0, 		x0, 		y0 + size, 	color);
}


bool ADESP_HX8357::tft_output2(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
   // Stop further decoding as image is running off bottom of screen
  if ( y >= height() ) return 0;

  // In ILI9341 library this function clips the image block at TFT boundaries
  drawRGBBitmap(x, y, bitmap, w, h);

  // Return 1 to decode next block
  return 1;
}


void ADESP_HX8357::drawJpeg(fs::File file, int xpos, int ypos) {

	TJpgDec.drawFsJpg(xpos, ypos, file);
}

void ADESP_HX8357::drawFSJpeg(fs::FS &fs, const char *filename, int xpos, int ypos) {
	
	File file = fs.open(filename);

	if ( !file ) {
		Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
		return;
	}

	TJpgDec.drawFsJpg(xpos, ypos, file);

}