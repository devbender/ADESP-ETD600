#pragma once

#include <ADESP_UI_Types.h>
#include <ADESP_Aircraft.h>
#include <ADESP_Airport.h>
#include <ADESP_UI_Elements.h>

//#define AVOID_SPLASH_SCREEN
//#define AVOID_WARNING_SCREEN

enum buttonList_t {
	ZOOM_IN,
	ZOOM_OUT
};

//########################################################################################################
// UI WINDOW CLASS
//########################################################################################################
class ADESP_UI_Window {

	ADESP_UI_Button buttons[5];
};

//########################################################################################################
// UI CLASS
//########################################################################################################
class ADESP_UI {

public:		
    std::unordered_map<uint32_t, Aircraft> aircrafts;
	std::unordered_map<uint8_t, Airport> airports;
	std::unordered_map<uint8_t, ADESP_UI_Button> buttons;

private:
    uint16_t heading = 0;
    uint16_t prevRange = 0;
	SemaphoreHandle_t UIxMutex;

	ui_t activeUI = NULL_UI;
	point_t compass;

public:
    ADESP_UI();

	void init();
	void splashScreen(uint16_t);
	void warningScreen(uint16_t);

    void drawOverlay(void);	
	void drawBaseUI(const char*);
	void drawUIButtons(ui_t);	
	void setActiveUI(ui_t);
	ui_t getActiveUI(void);

	void setLocationIcon(bool);
	void clearLocationIcon(bool);

	void setBluetoothIcon(bool);
	void clearBluetoothIcon(bool);

	void addAircraftByXY(uint32_t ICAO_ADD, int16_t x, int16_t y);
	void addAircraftByLocation(uint32_t ICAO_ADD, location_t location, traffic_t type);	
	void addAircraftByVector(uint32_t ICAO_ADD, vector_t vector);
	void addAircraftByLocation(uint32_t ICAO_ADD, location_t location);
	void addAircraftByVector(uint32_t ICAO_ADD, vector_t vector, traffic_t type);
	
	void removeStaleAircrafts();

	
	void addAirport(uint8_t, Airport);
	void drawAirports(void);
	void removeAirports();

	void drawBattery(bool);	
	void measureBattery();	

	void drawTime(bool);
    void setTime(int, int, bool); // hour, min, am

	void drawCompass(bool);
	void drawCompassDirection(bool);
    void setCompass(int16_t, bool); // heading	    
    
	void setRange(uint16_t); // new range

    uint8_t processInput(point_t);  // touch screen input point    
	void addButton(uint8_t, ADESP_UI_Button); // button ID, button obj
};