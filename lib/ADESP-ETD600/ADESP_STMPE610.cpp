#include "ADESP_STMPE610.h"


//#################################################################################################################
// *CLASS* >>> TFT TOUCH DRIVER SUBCLASS METHODS
//#################################################################################################################

ADESP_STMPE610::ADESP_STMPE610(int CS, int STMPE_INT, isr_t ISR) 
	: Adafruit_STMPE610(CS), INT_PIN(STMPE_INT), ISR(ISR) {

	pinMode(STMPE_INT, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(STMPE_INT), ISR, RISING);
}


void ADESP_STMPE610::clearBuffer(void) {
	uint8_t zR;
	uint16_t xR, yR;

	/* Clear touch buffer */
	while (!bufferEmpty()) 
		readData(&xR, &yR, &zR);
}

void ADESP_STMPE610::interruptReset(void) {
	
	if(bufferEmpty()) 
		writeRegister8(STMPE_INT_STA, 0xFF);
	
	setISRflag(false);	
}

void ADESP_STMPE610::enableISR(uint8_t intPin, isr_t ISR) {

	pinMode(intPin, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(intPin), ISR, RISING);
}

bool ADESP_STMPE610::ISR_Flag(void){
	return ISRflag;
}

void ADESP_STMPE610::setISRflag(bool flagVal){
	ISRflag = flagVal;
}

TS_Point ADESP_STMPE610::getTSPoint() {
	
	uint8_t zR;
	uint16_t xR = 0, yR = 0;
	int16_t tsX = 0, tsY = 0;

	while( bufferSize() > 0 ) {
		readData(&xR, &yR, &zR);

		if( bufferSize() == 1 ) {

			tsX = map(yR, TS_MINX, TS_MAXX, 0, TFT_WIDTH2 );
			tsY = map(xR, TS_MINY, TS_MAXY, 0, TFT_HEIGHT2);
		}
	}

	// Interrupt Reset
	interruptReset();
	
	// Reset touch flag
	setISRflag(false);

	if(tsX > 0 && tsY > 0) {
		return { tsX, tsY, 0 };
	}

	return { 0, 0, 0};
}

