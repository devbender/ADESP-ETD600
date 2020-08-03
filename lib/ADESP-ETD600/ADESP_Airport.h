#pragma once

#include <ADESP_UI_Types.h>
#include <ADESP_Aircraft.h>

#define LOG_1VAR(LABEL, VAR)		Serial.print(LABEL); Serial.print(" : "); Serial.println(VAR);

enum airport_t {
	AIRPORT,
	HELIPAD
};

//#######################################################################
// *CLASS* >>> AIRPORT CLASS DEFINITION
//#######################################################################
class Airport: public Aircraft {

public:
	static int index;

private:
	char icao[4];
	point_t pos;
	location_t location;
	uint16_t posDistance = 0;	
	airport_t type = AIRPORT;

public:
	Airport();	
	Airport(const char*, location_t);
	
	void draw(void);
	void draw(const char*, point_t);
	void erase(void);
	void update(void);
	void getXY();
};