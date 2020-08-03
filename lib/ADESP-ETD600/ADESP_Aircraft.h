#pragma once

#include <ADESP_UI_Types.h>

//#######################################################################
// *CLASS* >>> AIRCRAFT CLASS DEFINITION
//#######################################################################
class Aircraft {

public:
	Aircraft();
	Aircraft(vector_t);
	Aircraft(location_t);
	Aircraft(vector_t, traffic_t);
	Aircraft(location_t, traffic_t);
	
public:
	void draw(void);
	void erase(void);
	void update(void);
	
	void rotate(uint16_t);
	void setVector(vector_t);
	void setLocation(location_t);	

	uint16_t getLastSeen(void);
	vector_t getVector(void);
	location_t getLocation(void);

private:
	int16_t lbl;
	vector_t vector;
	location_t location;

	traffic_t type = ADSB;

	bool locationSet = false;
	bool vectorSet = false;

	point_t pos, vec;
	uint16_t posDistance = 0;
	uint16_t vectDistance = 0;
	uint16_t lastSeen = 0;
	double rotated = 0;

private:
	void getVectorXY();

//protected:	
	void getXY();
	
};