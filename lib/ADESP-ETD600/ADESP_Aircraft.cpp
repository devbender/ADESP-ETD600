#include "ADESP_Aircraft.h"

//#######################################################################
// GLOBALS
//#######################################################################
extern ADESP_HX8357 tft;
extern ADESP_STMPE610 ts;
extern program_states_t statesADESP;
extern mydata_t my;

#define TFT_WIDTH	tft.width()
#define TFT_HEIGHT	tft.height()

#define TFT_X_CENTER tft.width()/2
#define TFT_Y_CENTER tft.height()/2

#define TFT_DRAWABLE ( TFT_HEIGHT / 2 ) - AIRCRAFT_SIZE
#define TFT_LBL_DRAWABLE ( TFT_HEIGHT / 2 ) - ( AIRCRAFT_SIZE + 15 )

static int seconds() {
	return round(millis()/1000);
}

//#################################################################################################################################
// *CLASS* >>> AIRCRAFT CLASS
//#################################################################################################################################
Aircraft::Aircraft(){}

Aircraft::Aircraft(location_t _location) {	
	lastSeen = seconds();
	location = _location;
	locationSet = true;

	getXY();
	draw();
}

Aircraft::Aircraft(vector_t _vector) {
	lastSeen = seconds();
	vector = _vector;
	getVectorXY();

	vectorSet = true;
}

Aircraft::Aircraft(location_t _location, traffic_t _type) {	
	lastSeen = seconds();

	location = _location;
	locationSet = true;

	type = _type;

	getXY();
	draw();
}

Aircraft::Aircraft(vector_t _vector,  traffic_t _type) {
	lastSeen = seconds();
	
	vector = _vector;
	vectorSet = true;
	
	type = _type;
	
	getVectorXY();	
}

//=================================================================================================================
// *METHOD* >> DRAW ARICRAFT TO TFT
// *DESC* 	>> 
// *PARAMS*	>> None
// *TODO* >> Change spacing alt label when vrate 0
//=================================================================================================================
void Aircraft::draw() {

	if(!statesADESP.drawTraffic)
		return;

	// If location not set >> return
	if(!locationSet || pos.x == 0 || pos.y == 0)
		return;	

	// If location outside of drawing area >> return
	if(posDistance >= TFT_DRAWABLE)
		return;
	
	// Draw aircraft by type
	if(type == ADSB) 		tft.drawRhomb(pos.x, pos.y, AIRCRAFT_SIZE, WHITE);
	else if(type == MLAT) 	tft.drawCircle(pos.x, pos.y, AIRCRAFT_SIZE-1, WHITE);
	else if(type == TISB) 	tft.fillRect(pos.x-AIRCRAFT_SIZE/2, pos.y-AIRCRAFT_SIZE/2, AIRCRAFT_SIZE, AIRCRAFT_SIZE, GREY);
	else ; // pass		

	// If vector outside of drawing area >> return
	if(vectDistance >= TFT_DRAWABLE)
		return;
	
	// Draw aircraft Vector if set
	if(vectorSet) tft.drawLine(pos.x, pos.y, vec.x, vec.y, GREEN);
	
	// Draw Altitude Label in FL format
	lbl = round((float)location.alt/100);
	tft.setTextSize(1); tft.setTextColor(WHITE);

	if(posDistance <= TFT_LBL_DRAWABLE) {
		if(vectorSet) {
			if(vector.vrate >= 100) {
				tft.fillCircle(pos.x + 12, pos.y, 2, SKYBLUE);
				tft.setCursor(pos.x + 5, pos.y + 5); tft.setTextColor(WHITE);
				tft.print(lbl);
			}
			else if(vector.vrate <= -100) {				
				tft.fillCircle(pos.x + 12, pos.y, 2, ORANGE);
				tft.setCursor(pos.x + 5, pos.y + 5); tft.setTextColor(WHITE);
				tft.print(lbl);
			}
			else {
				tft.setCursor(pos.x + 5, pos.y + 5); tft.setTextColor(WHITE);
				tft.print(lbl);
			}
		} else {			
			tft.setCursor(pos.x + 5, pos.y + 5);
			tft.print(lbl);
		}
	}
}



//=================================================================================================================
// *METHOD* >> ERASE AIRCRAFT FROM TFT
// *DESC* 	>> 
// *PARAMS*	>> None
// *TODO* >> Handle different spacing when vrate 0 line 888
//=================================================================================================================
void Aircraft::erase() {

	if(!statesADESP.drawTraffic)
		return;

	// If location not set >> return
	if(!locationSet || pos.x == 0 || pos.y == 0)
		return;
	
	// If location set and outside drawing area >> return
	if(locationSet && posDistance >= TFT_DRAWABLE)
		return;

	// Erase aircraft
	tft.fillCircle(pos.x, pos.y, AIRCRAFT_SIZE, BLACK);	

	// If vector set and outside drawing area >> return
	if(vectorSet && vectDistance >= TFT_DRAWABLE)
		return;

	// Erase vector	
	if(vectorSet) tft.drawLine(pos.x, pos.y, vec.x, vec.y, BLACK);

	/* Erase Altitude Label */
	if(posDistance <= TFT_LBL_DRAWABLE) {		
		tft.setTextSize(1); tft.setTextColor(BLACK);
		if(vectorSet) {	

			if(vector.vrate >= 100) {				
				tft.fillCircle(pos.x + 12, pos.y, 2, BLACK);
				tft.setCursor(pos.x + 5, pos.y + 5);
				tft.print(lbl);
			}
			else if(vector.vrate <= -100) {				
				tft.fillCircle(pos.x + 12, pos.y, 2, BLACK);
				tft.setCursor(pos.x + 5, pos.y + 5);
				tft.print(lbl);
			}
			else {
				tft.setCursor(pos.x + 5, pos.y + 5);
				tft.print(lbl);
			}
		}
		else {			
			tft.setCursor(pos.x + 5, pos.y + 5);
			tft.print(lbl);
		}

	}
}



//=================================================================================================================
// *METHOD* >> AIRCRAFT LOCATION SETTER
// *DESC* 	>> 
// *PARAMS*	>> Location struct {altitude, latitude, longitude}
//=================================================================================================================
void Aircraft::setLocation(location_t newLocation) {
	
	lastSeen = seconds();	
	
	if(newLocation != location ) {
		
		if(locationSet) erase();
		else locationSet = true;

		location = newLocation;

		getXY();		
		if(vectorSet) getVectorXY();
		draw();
	}
}



//=================================================================================================================
// *METHOD* >> AIRCRAFT VECTOR SETTER
// *DESC* 	>> 
// *PARAMS*	>> Vector struct {speed, heading, vrate}
//=================================================================================================================
void Aircraft::setVector(vector_t newVector) {

	lastSeen = seconds();	
	
	if(newVector != vector ) {		
		
		if(locationSet) erase();
		vectorSet = true;

		vector = newVector;

		getVectorXY();
		
		if(locationSet) draw();
	}
}



//=================================================================================================================
// *METHOD* >> GET AIRCRAFT TFT CENTER POSITION X,Y
// *DESC* 	>> Calculates the aircraft's screen X,Y position from LAT & LON using an equirectangular approximation.
//			   This calculation uses the user's latitude and longitude as a reference point.
// *REF*	>> https://www.movable-type.co.uk/scripts/latlong.html
// *PARAMS*	>> None
//=================================================================================================================
void Aircraft::getXY() {	
	
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



//=================================================================================================================
// *METHOD* >> GET AIRCRAFT VECTOR END POSITION X,Y
// *DESC* 	>> 
// *PARAMS*	>> None
//=================================================================================================================
void Aircraft::getVectorXY() {

	uint8_t vectorLength = vector.speed / VECTOR_SIZE;
    
	float vectorX = ( cos( radians( vector.heading - 90 ) ) * vectorLength ) + pos.x;
	float vectorY = ( sin( radians( vector.heading - 90 ) ) * vectorLength ) + pos.y;

	vec.x = round( vectorX );
	vec.y = round( vectorY );

	float vdist = sqrt( pow(vec.x - TFT_X_CENTER, 2) + pow(vec.y - TFT_Y_CENTER, 2) );
	vectDistance = round(vdist);
}



//=================================================================================================================
// *METHOD* >> UPDATE AIRCRAFT TFT X,Y POSITION AND VECTOR
// *DESC* 	>> Recalculates the aircraft's screen XY position and vector when range changes
// *PARAMS*	>> None
//=================================================================================================================
void Aircraft::update() {	
	if(locationSet) {
		erase();
		getXY();
	}

	if(vectorSet) getVectorXY();
	if(locationSet) draw();
}



//=================================================================================================================
// *METHODS*>> OTHER GETTERS
// *DESC* 	>> 
// *PARAMS*	>> None
//=================================================================================================================
uint16_t Aircraft::getLastSeen(void) {
	return lastSeen;
}

vector_t Aircraft::getVector(void) {
	return vector;
}

location_t Aircraft::getLocation(void) {
	return location;
}