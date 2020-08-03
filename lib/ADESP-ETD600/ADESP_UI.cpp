#include "ADESP_UI.h"

//#######################################################################
// GLOBALS
//#######################################################################
extern ADESP_HX8357 tft;
extern ADESP_STMPE610 ts;
extern program_states_t statesADESP;

extern mydata_t my;

extern TaskHandle_t xHandleInitTask, xHandleMainTask, xHandleEventsTask, xHandleOverlayTask;

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
// *CLASS* >>> ADESP UI CLASS
//#################################################################################################################################
ADESP_UI::ADESP_UI() {}

void ADESP_UI::init() {

	// Initialize TFT
	tft.begin();	
	tft.setRotation(3);
	tft.fillScreen(BLACK);

	// Init UI Mutex
	UIxMutex = xSemaphoreCreateMutex();
	
	// Display splash/warining screens
	splashScreen(3000);
	warningScreen(5000);

	// UI Buttons --------------------------------------------------------
	addButton(1, ADESP_UI_Button("AIRPORTS", 10, 190, 60, 50) );
	buttons[1].setType(TOGGLE);
	buttons[1].setFillColor( UI_BUTTON_FILL_COLOR );
	buttons[1].assignUI(TRAFFIC_UI);	
	
	addButton(2, ADESP_UI_Button("DIM LCD", 10, 120, 60, 50) );
	buttons[2].setType(TOGGLE);
	buttons[2].setFillColor( UI_BUTTON_FILL_COLOR );
	buttons[2].assignUI(TRAFFIC_UI);

	addButton(3, ADESP_UI_Button("+", 410, 40, 60, 120) );
	buttons[3].setType(MOMENTARY);
	buttons[3].setFillColor( UI_BUTTON_FILL_COLOR );
	buttons[3].setLabelTextSize(3);
	buttons[3].assignUI(TRAFFIC_UI);

	addButton(4, ADESP_UI_Button("-", 410, 190, 60, 120) );
	buttons[4].setType(MOMENTARY);
	buttons[4].setFillColor( UI_BUTTON_FILL_COLOR );
	buttons[4].setLabelTextSize(3);
	buttons[4].assignUI(TRAFFIC_UI);

	addButton(5, ADESP_UI_Button("HOME", 10, 260, 60, 50) );
	buttons[5].setType(MOMENTARY);
	buttons[5].setFillColor( UI_BUTTON_FILL_COLOR );
	buttons[5].assignUI(COMMON);
	buttons[5].setPressedColor(BLUE);

	// SETTINGS ---------------------------------------------------------------	

	addButton(10, ADESP_UI_Button("DIM LCD", 130, 40, 80, 40) );
	buttons[10].setType(SWITCH);
	buttons[10].setFillColor( UI_BUTTON_FILL_COLOR );	
	buttons[10].assignUI(SETTINGS_UI);

	addButton(11, ADESP_UI_Button("BLUETOOTH", 130, 110, 80, 40) );
	buttons[11].setType(SWITCH);
	buttons[11].setFillColor( UI_BUTTON_FILL_COLOR );	
	buttons[11].assignUI(SETTINGS_UI);

	addButton(12, ADESP_UI_Button("DEBUG TOUCH", 130, 180, 80, 40) );
	buttons[12].setType(SWITCH);
	buttons[12].setFillColor( UI_BUTTON_FILL_COLOR );	
	buttons[12].assignUI(SETTINGS_UI);

	// ICONS ---------------------------------------------------------------	
	addButton(6, ADESP_UI_Button("TRAFFIC", 16, 50, 100, 100) );	
	buttons[6].setType(ICON);
	buttons[6].setOutlineColor(BLACK);
	buttons[6].setImage("/Picture1.jpg");	
	buttons[6].assignUI(MAIN_UI);

	addButton(7, ADESP_UI_Button("WEATHER", 132, 50, 100, 100) );	
	buttons[7].setType(ICON);
	buttons[7].setOutlineColor(BLACK);
	buttons[7].setImage("/Picture2.jpg");	
	buttons[7].assignUI(MAIN_UI);	

	addButton(8, ADESP_UI_Button("MAP", 248, 50, 100, 100) );	
	buttons[8].setType(ICON);
	buttons[8].setOutlineColor(BLACK);
	buttons[8].setImage("/Picture3.jpg");
	buttons[8].assignUI(MAIN_UI);

	addButton(9, ADESP_UI_Button("SETTINGS", 364, 50, 100, 100) );	
	buttons[9].setType(ICON);
	buttons[9].setOutlineColor(BLACK);
	buttons[9].setImage("/Picture4.jpg");
	buttons[9].assignUI(MAIN_UI);	

	// Battery
	measureBattery();
	drawBattery(true);
}


void ADESP_UI::splashScreen(uint16_t timeMS) {

	/* Avoid Compiler Warning */
	#ifdef AVOID_SPLASH_SCREEN
		return;
	#endif

	tft.fillScreen(BLACK);
	//tft.drawFSJpeg(SD,"/test.jpg", 0, 0);

	tft.setTextSize(3);
	tft.setTextColor(WHITE);
	tft.printCenteredAbs(SYSTEM_NAME);	

	char buff[32];
	sprintf(buff, "VERSION: %s", ADESP_VER);

	tft.setTextSize(1);		
	tft.printCenteredAbs(buff, 0, 25);

	// Fade in/out screen
	tft.fadeInOut(timeMS);
}

void ADESP_UI::warningScreen(uint16_t timeMS) {
	
	/* Avoid Compiler Warning */
	#ifdef AVOID_WARNING_SCREEN
		return;
	#endif
	
	tft.fillScreen(BLACK);
	tft.setTextSize(3); tft.setTextColor(RED);
	tft.printCenteredAbs("WARNING", 0, -60);

	tft.setTextSize(2); tft.setTextColor(WHITE);
	tft.printCenteredAbs("STRICTLY FOR EXPERIMENTAL USE", 0, -20);
	tft.printCenteredAbs("NOT INTENDED FOR REAL NAVIGATION", 0, 20);
	tft.printCenteredAbs("USE AT YOUR OWN RISK!", 0, 60);

	// Fade in/out screen
	tft.fadeInOut(timeMS);
}

void ADESP_UI::drawBaseUI(const char* UI_TITTLE) {
	
	// Background
	tft.fillScreen(UI_BACKGROUND_COLOR);	
		
	// Top Black Bar
	tft.fillRect(0,0, tft.width(), 30, UI_TOP_BAR_COLOR);	
	tft.drawFastHLine(0, 30, tft.width(), UI_TOP_BAR_DIV_LINE_COLOR);

	// Top bar text
	tft.setFont(&CalibriBold8pt7b);
	tft.setTextSize(1); tft.setTextColor(WHITE);
	 
	// System Name
	tft.setCursor(10, 18); tft.print(UI_TITTLE);

	// Restore Default Font
	tft.setFont();
	tft.setTextSize(1);
	tft.setTextColor(WHITE);


	setLocationIcon(false);
	
	if(statesADESP.BLUETOOTH == ACTIVE)
		setBluetoothIcon(false);

}

void ADESP_UI::drawUIButtons(ui_t UI) {
	// Iterate through buttons
	for(auto& button : buttons) {		
		
		// If button is assigned to active UI process touch input
		if( UI == button.second.getAssignedUI() || 
		  (button.second.getAssignedUI() == COMMON && UI != MAIN_UI) ) {
			
			button.second.draw();

			if(button.second.getType() == TOGGLE || button.second.getType() == SWITCH)
				button.second.drawToggled(button.second.isActive());
		}
	}
}

//==========================================================================================
// CHANGE UI
//==========================================================================================
void ADESP_UI::setActiveUI(ui_t UI) {

	// Set new UI if changed
	if(activeUI == UI) return;
	else activeUI = UI;

	// Fade out
	tft.fadeOut();

	// Lock UI
	xSemaphoreTake( UIxMutex, portMAX_DELAY);

	// DRAW UI SPECIFICS
	switch(UI) {
		
		//---------------------------------------------------------------------------------
		// MAIN UI
		//---------------------------------------------------------------------------------
		case MAIN_UI: {
			
			// Suspend traffic and overlay drawing
			vTaskSuspend( xHandleOverlayTask );
			statesADESP.drawTraffic = false;

			// Base UI with Tittle
			drawBaseUI("ADESP-ETD600");
			
			break;
		}

		//---------------------------------------------------------------------------------
		// TRAFFIC UI
		//---------------------------------------------------------------------------------
		case TRAFFIC_UI: {

			// Base UI with Tittle
			drawBaseUI("TRAFFIC");

			// Resume overlay/traffic drawing
			vTaskResume( xHandleOverlayTask );
			statesADESP.drawTraffic = true;

			// Draw Compass
			drawCompass(false);
			setCompass(-my.vector.heading, false);

			// Traffic drawing area
			tft.fillCircle(tft.width()/2, tft.height()/2, tft.height()/2-1, BLACK);

			// Draw outer circle
			tft.drawCircle(tft.width()/2,  tft.height()/2, tft.height()/2-1, UI_OUTER_CIRCLE_COLOR);	
				
			// Draw range text 
			int offset = 12;	
			tft.setTextSize(2); tft.setTextColor(WHITE);	
			//tft.setCursor(tft.width()/8, tft.height() - tft.height()/10);
			if(statesADESP.currentRange < 100) 	
				tft.setCursor(tft.width() - tft.width()/3 + 18 + offset, tft.height() - tft.height()/10);
			else 
				tft.setCursor(tft.width() - tft.width()/3 + 18, tft.height() - tft.height()/10);

			tft.print(statesADESP.currentRange);

			// Print Units			
			tft.setCursor(tft.width() - tft.width()/3 + 58, tft.height() - tft.height()/10);
			tft.print("Nm");
			
			break;
		}

		//---------------------------------------------------------------------------------
		// WEATHER UI
		//---------------------------------------------------------------------------------
		case WEATHER_UI: {

			// Base UI with Tittle
			drawBaseUI("WEATHER");

			// Message	
			tft.setTextSize(2); tft.setTextColor(WHITE);	
			tft.printCenteredAbs("Weather/METARs comming soon...");
			
			break;
		}

		//---------------------------------------------------------------------------------
		// MAP UI
		//---------------------------------------------------------------------------------
		case MAP_UI: {

			// Base UI with Tittle
			drawBaseUI("MAP");

			// Draw sample map
			tft.drawFSJpeg(SD,"/map.jpg", 0,31);

			break;
		}

		//---------------------------------------------------------------------------------
		// SETTINGS UI
		//---------------------------------------------------------------------------------
		case SETTINGS_UI: {

			// Base UI with Tittle
			drawBaseUI("SETTINGS");			

			break;
		}



		//---------------------------------------------------------------------------------
		// DEFAULT
		//---------------------------------------------------------------------------------
		default:
			break;
	}

	// Draw buttons
	drawUIButtons(UI);

	// Battery
	drawBattery(false);

	// Unlock
	xSemaphoreGive( UIxMutex);

	// Fade in
	tft.fadeIn();	
}

ui_t ADESP_UI::getActiveUI() {
	return activeUI;
}

void ADESP_UI::addButton(uint8_t id, ADESP_UI_Button button){
    buttons.insert( { id, button } );
}

uint8_t ADESP_UI::processInput(point_t touchPoint) {

	// Iterate through buttons
	for(auto& button : buttons) {
		
		// If button is assigned to active UI process touch input
		if( activeUI == button.second.getAssignedUI() ||
			(button.second.getAssignedUI() == COMMON && activeUI != MAIN_UI)) {

			// If touch point inside button region
			if(touchPoint == button.second.getRegion()) {
				
				// If button is momentary give momentary visual feedback
				if(button.second.getType() == MOMENTARY || button.second.getType() == ICON) 
					button.second.momentaryPress();				

				// If button is toggle type check and set active status
				else if(button.second.getType() == TOGGLE || button.second.getType() == SWITCH ) {
					
					if(button.second.isActive()) button.second.setInactive();
					else button.second.setActive();
				}
				
				// Return ID of pressed button (uint8_t)
				return button.first;
			}
		}
    }
	
	// if touch input dont match any parameters return 255
    return 255;
}


void ADESP_UI::setLocationIcon(bool useLock) {

	// Lock
	if(useLock) xSemaphoreTake( UIxMutex, portMAX_DELAY);

	tftPoint_t p{380, 12};

    tft.fillCircle(p.x, p.y, 5, RED);
	tft.fillTriangle(p.x-5,p.y, p.x+5,p.y, p.x,p.y+10, RED);
	tft.fillCircle(p.x, p.y, 2, BLACK);	

	// Unlock
	if(useLock) xSemaphoreGive( UIxMutex );

}


void ADESP_UI::setBluetoothIcon(bool useLock) {

	// Lock
	if(useLock) xSemaphoreTake( UIxMutex, portMAX_DELAY);

	// Icon location
	tftPoint_t p{395, 4};

	// Build bluetooth icon out of basic shapes
	tft.fillRoundRect(p.x, p.y, 14, 21, 12, BLUE);
	
	tft.drawTriangle(p.x +  6,	p.y +  4, 
					 p.x +  6,	p.y + 12, 
					 p.x + 10,	p.y +  8, WHITE);	
	
	tft.drawTriangle(p.x +  6,	p.y +  9, 
					 p.x +  6,	p.y + 17, 
					 p.x + 10,	p.y + 13, WHITE);
	
	tft.drawLine(p.x + 6,	p.y + 12, 
				 p.x + 3,	p.y +  8, WHITE);	
	
	tft.drawLine(p.x + 6,	p.y +  9, 
				 p.x + 3,	p.y + 13, WHITE);    

	// Unlock
	if(useLock) xSemaphoreGive( UIxMutex );

}

void ADESP_UI::clearBluetoothIcon(bool useLock) {
	// Lock
	if(useLock) xSemaphoreTake( UIxMutex, portMAX_DELAY);

	// Icon location
	tftPoint_t p{395, 4};

	// Erase BT icon
	tft.fillRoundRect(p.x, p.y, 14, 21, 12, BLACK);
	
	// Unlock
	if(useLock) xSemaphoreGive( UIxMutex );

}


void ADESP_UI::measureBattery() {	

	int battVal = 0;

	// Take X samples every Y ms
	for (int i = 0; i < BATT_SAMPLES; i++){
		battVal += analogRead(A13);
		vTaskDelay(BATT_SAMPLES_DELAY / portTICK_PERIOD_MS);
	}

	// Average Measurements
	double battAVG = (2 * (battVal / BATT_SAMPLES));

	// Map battery voltage to %
	int batteryPercent = round(map(battAVG, BATT_MIN, BATT_MAX, 0, 100));

	// Limit percent range
	if(batteryPercent < 0) batteryPercent = 0;
    if(batteryPercent > 100) batteryPercent = 100;
	
	// Save Battery %
	statesADESP.battery = batteryPercent;
}

void ADESP_UI::drawBattery(bool useLock) {

    // Set battery level color 
    uint16_t color;    
    if(statesADESP.battery >= 60) color = GREEN;
    if(statesADESP.battery < 60 && statesADESP.battery > 40) color = YELLOW2;
    if(statesADESP.battery <= 40) color = RED;

	// Map battery% to pixels
    uint16_t battPx = round( map(statesADESP.battery, 0, 100, 0, 50) );

	// Acquire ui/tft lock
	if(useLock) xSemaphoreTake( UIxMutex, portMAX_DELAY);

    // Erase battery indicator
	tft.fillRect(420, 6, 50, 16, BLACK);
	
	// Draw new battery indicator
    tft.fillRect(420, 6, battPx, 16, color);
	tft.drawRect(420, 6, 50, 16, WHITE);

	// Show measured voltage	
	tft.setTextSize(1);
	tft.setTextColor(BLACK);
	tft.setCursor(425, 10);
	
	// tft.print( (battAVG/1024) - 0.5 ); //voltage
	//tft.setCursor(450, 10);
	//tft.print('v');

	tft.print( statesADESP.battery ); //voltage
	if(statesADESP.battery == 100) tft.setCursor(445, 10);
	if(statesADESP.battery < 100) tft.setCursor(440, 10);
	tft.print('%');

	// Unlock
	if(useLock) xSemaphoreGive( UIxMutex );
}


void ADESP_UI::drawCompass(bool useLock) {

	// Lock
	if(useLock) xSemaphoreTake( UIxMutex, portMAX_DELAY);

    tft.fillCircle(45, 70, 35, BLACK);
	tft.fillCircle(45, 70, 2, RED);
	tft.drawCircle(45, 70, 35, UI_OUTER_CIRCLE_COLOR);
    
	// Set compass initial position
	compass.x = 45;
	compass.y = 50;
	tft.fillCircle(compass.x, compass.y, 3, WHITE);

	// Unlock
	if(useLock) xSemaphoreGive( UIxMutex );
}

void ADESP_UI::setCompass(int16_t a, bool useLock) {	

	// Compass Center
    int cX = 45;
    int cY = 70;

	// Starting point (NORTH)
	int px = 0;
	int py = 20;

	// Rotation Matrix
	float s = sin(radians(a));
	float c = cos(radians(a));
  
	float xR = (px*c + py*s) + cX;
	float yR = (px*s - py*c) + cY;

	// Lock
	if(useLock) xSemaphoreTake( UIxMutex, portMAX_DELAY);

	// Erase previous position
	tft.fillCircle(compass.x, compass.y, 3, BLACK);

    // Draw new rotated position
	tft.fillCircle(xR , yR, 3, WHITE);

	// Save new position
	compass.x = xR;
	compass.y = yR;

	// Unlock
	if(useLock) xSemaphoreGive( UIxMutex );

}


void ADESP_UI::setRange(uint16_t newRange) {
    
	// Lock
	xSemaphoreTake( UIxMutex, portMAX_DELAY);
	
	int offset = 12;
	tft.setTextSize(2);
	tft.setTextColor(WHITE);
	
	/* Increase Range (Zoom Out) */
	if(newRange != statesADESP.currentRange) { 

		// Erase old range text		
		tft.eraseText(statesADESP.currentRange, tft.width() - tft.width()/3 + 18, tft.height() - tft.height()/10, UI_BACKGROUND_COLOR);

		// Enforce range limits
		if(newRange < MIN_RANGE) newRange = MIN_RANGE;
		if(newRange > MAX_RANGE) newRange = MAX_RANGE;
		
		// Align text with units when range < 2 digits
		if(newRange >= 100) 
			tft.setCursor(tft.width() - tft.width()/3 + 18, tft.height() - tft.height()/10);			
		else 
			tft.setCursor(tft.width() - tft.width()/3 + 18 + offset, tft.height() - tft.height()/10);

		// Draw and save range text
		tft.print(newRange);
		statesADESP.currentRange = newRange;

		// Redraw airports
		for(auto& airport : airports) {
			if(statesADESP.drawAirports) 
				airport.second.update();			
			else
				airport.second.getXY();			
		}
		
		// Redraw aircrafts with new range
		for(auto& aircraft : aircrafts) {
			aircraft.second.update();			
		}		
	}

	// Unlock
	xSemaphoreGive( UIxMutex );
}


void ADESP_UI::drawOverlay() {	

	// Draw only in Traffic UI
	if(getActiveUI() != TRAFFIC_UI)
		return;

	// Lock
    xSemaphoreTake( UIxMutex, portMAX_DELAY);
		
	// Draw outer range circle
	tft.drawCircle(TFT_X_CENTER, TFT_Y_CENTER, TFT_HEIGHT/2-1, UI_OUTER_CIRCLE_COLOR);	

	// Inner Circle Params
	int degMark = 15;
	int ri = tft.height() / 4;
	
	// Draw inner circle
	for (int i = 0; i<360; i += degMark) {			
		float a = radians(i);
		tft.drawPixel(TFT_X_CENTER + cos(a)*ri, TFT_Y_CENTER + sin(a)*ri, UI_INNER_CIRCLE_COLOR);
	}

	// Draw My Center Position / Aircraft
	if(statesADESP.northUp) 
		tft.drawCircle(tft.width()/2, tft.height()/2, 3, RED);
	else {
		tft.drawLine(TFT_X_CENTER-AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, TFT_X_CENTER,				TFT_Y_CENTER-AIRCRAFT_SIZE, UI_MY_AIRCRAFT_COLOR);
		tft.drawLine(TFT_X_CENTER,					TFT_Y_CENTER-AIRCRAFT_SIZE, TFT_X_CENTER+AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, UI_MY_AIRCRAFT_COLOR);		
		tft.drawLine(TFT_X_CENTER-AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, TFT_X_CENTER,				TFT_Y_CENTER+3, 			UI_MY_AIRCRAFT_COLOR);
		tft.drawLine(TFT_X_CENTER+AIRCRAFT_SIZE,	TFT_Y_CENTER+AIRCRAFT_SIZE, TFT_X_CENTER,				TFT_Y_CENTER+3, 			UI_MY_AIRCRAFT_COLOR);
	}

	// Inner Circle Range Label
	tft.setTextSize(1); tft.setTextColor(WHITE);

	// Erase previous range if changed
	if(statesADESP.currentRange/2 != prevRange)
		tft.eraseText("000",  TFT_X_CENTER - 20, TFT_Y_CENTER + 90, BLACK);
	
	if(statesADESP.currentRange/2 < 10) {			
		tft.setCursor(TFT_X_CENTER - 10, TFT_Y_CENTER + 90); tft.print(statesADESP.currentRange/2);
		tft.setCursor(TFT_X_CENTER -  0, TFT_Y_CENTER + 90); tft.print("Nm");
	} else if(statesADESP.currentRange/2 > 99) {
		//tft.eraseText("000",  TFT_X_CENTER - 20, TFT_Y_CENTER + 90, BLACK);
		tft.setCursor(TFT_X_CENTER - 20, TFT_Y_CENTER + 90); tft.print(statesADESP.currentRange/2);
		tft.setCursor(TFT_X_CENTER -  0, TFT_Y_CENTER + 90); tft.print("Nm");
	} else {
		//tft.eraseText("000",  TFT_X_CENTER - 20, TFT_Y_CENTER + 90, BLACK);
		tft.setCursor(TFT_X_CENTER - 15, TFT_Y_CENTER + 90); tft.print(statesADESP.currentRange/2);
		tft.setCursor(TFT_X_CENTER -  0, TFT_Y_CENTER + 90); tft.print("Nm");
	}
	
	// Save current range
	prevRange = statesADESP.currentRange/2;

	// Redraw airports
	if(statesADESP.drawAirports) {
		for(auto& airport : airports) {
			airport.second.draw();		
		}
	}	

	// Redraw traffic	
	for(auto& aircraft : aircrafts) {
		aircraft.second.draw();
	}	

	// Unlock
	xSemaphoreGive( UIxMutex );
}


void ADESP_UI::addAircraftByLocation(uint32_t ICAO_ADD, location_t location) {

	// Lock
	xSemaphoreTake( UIxMutex, portMAX_DELAY);

	auto found = aircrafts.find(ICAO_ADD);

	if (found == aircrafts.end())  aircrafts.insert( { ICAO_ADD,  Aircraft(location) } );		
	else  aircrafts[ICAO_ADD].setLocation(location);

	// Unlock
	xSemaphoreGive( UIxMutex );

}

void ADESP_UI::addAircraftByVector(uint32_t ICAO_ADD, vector_t vector) {

	// Lock
	xSemaphoreTake( UIxMutex, portMAX_DELAY);
	
	auto found = aircrafts.find(ICAO_ADD);
	
	if (found == aircrafts.end())  {} //aircraft.emplace( ICAO_ADD,  Aircraft(vector) );
	else  aircrafts[ICAO_ADD].setVector(vector);

	// Unlock
	xSemaphoreGive( UIxMutex );

}

void ADESP_UI::addAircraftByLocation(uint32_t ICAO_ADD, location_t location, traffic_t type) {

	// Lock
	xSemaphoreTake( UIxMutex, portMAX_DELAY);

	auto found = aircrafts.find(ICAO_ADD);

	if (found == aircrafts.end())  aircrafts.insert( { ICAO_ADD,  Aircraft(location, type) } );		
	else  aircrafts[ICAO_ADD].setLocation(location);

	// Unlock
	xSemaphoreGive( UIxMutex );

}

void ADESP_UI::addAircraftByVector(uint32_t ICAO_ADD, vector_t vector, traffic_t type) {

	// Lock
	xSemaphoreTake( UIxMutex, portMAX_DELAY);
	
	auto found = aircrafts.find(ICAO_ADD);
	
	if (found == aircrafts.end())  aircrafts.insert( { ICAO_ADD,  Aircraft(vector, type) } );
	else  aircrafts[ICAO_ADD].setVector(vector);

	// Unlock
	xSemaphoreGive( UIxMutex );

}

void ADESP_UI::removeStaleAircrafts() {
	
	// Lock
	xSemaphoreTake( UIxMutex, portMAX_DELAY);

	// Get current running time
	auto runningTime = seconds();

	// Check each aircrafts TTL and remove if old
	for(auto& aircraft : aircrafts) {
		if(runningTime - aircraft.second.getLastSeen() > AIRCRAFT_TTL) {														
			aircraft.second.erase();			// erase from tft
			aircrafts.erase(aircraft.first); 	// remove from container
		}							
	}

	// Unlock
	xSemaphoreGive( UIxMutex );
}


void ADESP_UI::addAirport(uint8_t index, Airport a) {
	
	// Lock
	xSemaphoreTake( UIxMutex, portMAX_DELAY);
	
	airports.insert( {index, a} );

	// Unlock
	xSemaphoreGive( UIxMutex );
}

void ADESP_UI::drawAirports(void) {

	statesADESP.drawAirports = true;
	
	if(airports.size() > 0) {
		for(auto& airport : airports) {
			airport.second.draw();
		}			
	}
}

void ADESP_UI::removeAirports(void) {

	statesADESP.drawAirports = false;
	
	if(airports.size() > 0) {
		for(auto& airport : airports) {
			airport.second.erase();
		}			
	}
}