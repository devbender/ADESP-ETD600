#include "ADESP_Airport.h"

//#######################################################################
// GLOBALS
//#######################################################################
extern ADESP_HX8357 tft;
extern program_states_t statesADESP;
extern mydata_t my;

#define TFT_X_CENTER tft.width()/2
#define TFT_Y_CENTER tft.height()/2
#define TFT_WIDTH	tft.width()
#define TFT_HEIGHT	tft.height()

#define TFT_DRAWABLE ( TFT_HEIGHT / 2 ) - AIRPORT_SIZE

static inline tftPoint_t translateXY_toCenter(point_t p) {
	return { (uint16_t)(TFT_X_CENTER + p.x) , (uint16_t)(TFT_Y_CENTER - p.y)};
}

//#################################################################################################################################
// *CLASS* >>> AIRPORTS CLASS
//#################################################################################################################################

Airport::Airport() {}

Airport::Airport(const char* _icao, location_t _location) 
	: location(_location) { 
	strcpy(icao, _icao); 
	getXY();
	draw();
}

void Airport::draw(void) {

	if(!statesADESP.drawAirports)
		return;

	// If location set and outside drawing area >> return
	if(posDistance >= TFT_DRAWABLE)
		return;

	if(posDistance < 20)
		return;

	// Draw Airport
	tft.drawTriangle(pos.x - AIRPORT_SIZE, pos.y + AIRPORT_SIZE,
					 pos.x + AIRPORT_SIZE, pos.y + AIRPORT_SIZE,
					 pos.x, pos.y - AIRPORT_SIZE, RED);
}

void Airport::draw(const char* icaoID, point_t p) {

	tftPoint_t pt = translateXY_toCenter(p);

	tft.fillCircle(pt.x, pt.y, AIRPORT_SIZE, BLUE);
	tft.fillRect(pt.x-2, pt.y-7, 5, 16, WHITE);
}


void Airport::erase(void) {	
	
	// If location set and outside drawing area >> return
	if(posDistance >= TFT_DRAWABLE)
		return;
	
	if(posDistance < 20)
		return;

	// Erase Airport
	tft.drawTriangle(pos.x - AIRPORT_SIZE, pos.y + AIRPORT_SIZE,
					 pos.x + AIRPORT_SIZE, pos.y + AIRPORT_SIZE,
					 pos.x, pos.y - AIRPORT_SIZE, BLACK);
}

void Airport::getXY() {	
	
	float xF, yF, dF;
	
	/* Calculate cartesian X,Y from LAT & LON */
	xF = ((radians(location.lon) - radians(my.location.lon)) * cos((radians(my.location.lat) + radians(location.lat)) / 2)) * EARTH_RAD_NM;
	yF = (radians(location.lat) - radians(my.location.lat)) * EARTH_RAD_NM;
	dF = sqrt( xF*xF + yF*yF );
	
	/* Round and scale to selected range */
	pos.x = TFT_X_CENTER + round(xF * (tft.height()/2) / statesADESP.currentRange);
	pos.y = TFT_Y_CENTER - round(yF * (tft.height()/2) / statesADESP.currentRange);
	
	posDistance = round(dF * (tft.height()/2) / statesADESP.currentRange);
}


void Airport::update() {
	erase();
	getXY();
	draw();
}