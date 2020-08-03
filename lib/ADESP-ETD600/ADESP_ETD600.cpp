#include "ADESP_ETD600.h"
#include "ADESP_UI.h"
#include "ADESP_Aircraft.h"

//#######################################################################
// *GLOBALS
// *TODO:
//#######################################################################
extern ADESP_HX8357 tft;
extern ADESP_STMPE610 ts;
ADESP_UI ui;

BluetoothSerial SerialBT;
SemaphoreHandle_t xMutex;
QueueHandle_t xQueueEventRx, xQueueTouchEvents;

program_states_t statesADESP;
hw_timer_t *timer5s = NULL;

mydata_t my;
std::unordered_map<uint32_t, Aircraft> aircraftList;
TaskHandle_t xHandleInitTask, xHandleMainTask, xHandleEventsTask, xHandleTouchTask, xHandleOverlayTask;

int Airport::index = 0;

//#######################################################################
// *Fx: GET RUNTIME SECONDS
//#######################################################################
static int seconds() {
	return round(millis()/1000);
}


//#######################################################################
// *Fx: BLUETOOTH CALLBACK
//#######################################################################
void BT_Callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
	
	// Bluetooth client connected
	if(event == ESP_SPP_SRV_OPEN_EVT) {		
		LOG_INFO("BT CONNECTION | CLIENT_CONNECTED");
		
		digitalWrite(LED_BUILTIN, HIGH); // led ON
		statesADESP.BLUETOOTH = ACTIVE;
		ui.setBluetoothIcon(true);
	}
	
	// Bluetooth client disconnected
	if(event == ESP_SPP_CLOSE_EVT) {		
		LOG_INFO("BT CONNECTION | CLIENT_DISCONNECTED");

		digitalWrite(LED_BUILTIN, LOW); // led OFF
		statesADESP.BLUETOOTH = INACTIVE;
		ui.clearBluetoothIcon(true);
	}
	
	// Bluetooth data event
	if(event == ESP_SPP_DATA_IND_EVT) {

		if(SerialBT.available() == ADESP_FRAMESIZE) {			
			
			// Allocate frame and read bytes to frame buffer
			dataFrame_t xBTframe;	
			SerialBT.readBytes(xBTframe.buff, ADESP_FRAMESIZE);
			SerialBT.flush();
			
			// Process incomming frame
			ADESP_PROCESS_FRAME(xBTframe);
		}

		// Discard data if frame size mismatch
		else {
			LOG_1VAR("bytesRX", SerialBT.available());
			SerialBT.flush();
		}
	}
}


//#######################################################################
// *Fx >>> INTERRUPT HANDLING
//#######################################################################
void IRAM_ATTR onTS_TOUCH() {
	
	if(statesADESP.initComplete) 
		ts.setISRflag(true);
}

static void IRAM_ATTR on5sTimer() {
	eventData_t event;
	event.type = REMOVE_STALE_AIRCRAFT;
	xQueueSendFromISR( xQueueEventRx, (void*) &event, 0U );
}

static void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

//#######################################################################
// *TASK >>> TFT/UI INIT TASK
//#######################################################################
static void TASK_INIT(void *pvParameters) {

	// Block until initialization completes
	xSemaphoreTake( xMutex, portMAX_DELAY);

	// Init SPI Flash FS
	if(!SPIFFS.begin(true)) LOG_ERROR("INIT TASK | SPIFFS MOUNT FAILED");

	// Init SD Card FS
  	if(!SD.begin(14, SPI, 40000000)) LOG_ERROR("INIT TASK | SDCARD MOUNT FAILED");

	// Initialize TFT LCD & UI
	ui.init();
	LOG_INFO("INIT TASK | UI INITIALIZED");	
	
	// Initialize bluetooth and set callback 
	SerialBT.begin(SYSTEM_NAME);
	SerialBT.register_callback(BT_Callback);
	LOG_INFO("INIT TASK | BLUETOOTH SERVICE STARTED");
	
	// Enable Builtin LED
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW); // led OFF

	// Enable 5s timer interrupt
	timer5s = timerBegin(0, 80, true);
	timerAttachInterrupt(timer5s, &on5sTimer, true);
	timerAlarmWrite(timer5s, 5000000, true);
		timerAlarmEnable(timer5s);

	// Init Complete
	LOG_INFO("INIT TASK | INIT COMPLETE!");	
	xSemaphoreGive( xMutex );
	vTaskDelete(NULL); 
}


//#######################################################################
// *TASK >>> DRAW OVERLAY
//#######################################################################
static void TASKS_OVERLAY(void *pvParameters) {

	for(;;) {
		ui.drawOverlay();
		vTaskDelay(OVERLAY_DRAW_DELAY / portTICK_PERIOD_MS);
	}
}

//#######################################################################
// *TASK >>> TOUCH EVENTS
//#######################################################################
static void TOUCH_EVENTS(void *pvParameters) {
	
	for(;;) {
		
		// Read TS data if ISR flag was set
		if( ts.ISR_Flag() ) {

			uint8_t zR;
			uint16_t xR = 0, yR = 0;
			uint16_t tsX = 0, tsY = 0;

			while( ts.bufferSize() > 0 ) {
				ts.readData(&xR, &yR, &zR);

				if( ts.bufferSize() == 1 ) {

					tsX = map(yR, TS_MINX, TS_MAXX, 0, TFT_WIDTH2 );
					tsY = map(xR, TS_MINY, TS_MAXY, 0, TFT_HEIGHT2);
				}
			}

			// Interrupt Reset
			ts.interruptReset();
			
			// Grab input when available
			if(tsX > 0 && tsY > 0) {			

				// Process touch input
				uint8_t buttonID = ui.processInput( {tsX, tsY} );
				
				// Output TS debug info if enabled
				if(statesADESP.debugTouchInput) {
					tft.drawCircle(tsX, tsY, 2, RED);
					Serial.printf("%i - %i | %i - %i", tsX, yR, tsY, xR);					
				}				

				// Execute input action
				if(buttonID == 0) {}

				// BUTTON ID 1: AIRPORTS
				else if(buttonID == 1) {
					if( ui.buttons[buttonID].isActive() )
						ui.drawAirports();
					else
						ui.removeAirports();
				}
				
				// BUTTON ID 3: DIM LCD
				else if(buttonID == 2) {
					if( ui.buttons[buttonID].isActive() ) tft.setBRT(LCD_DIMMED);
					else tft.setBRT(LCD_ON);
				}
				else if(buttonID == 3) ui.setRange(statesADESP.currentRange - 10);
				else if(buttonID == 4) ui.setRange(statesADESP.currentRange + 10);
				else if(buttonID == 5) ui.setActiveUI(MAIN_UI);
				else if(buttonID == 6) ui.setActiveUI(TRAFFIC_UI);
				else if(buttonID == 7) ui.setActiveUI(WEATHER_UI);
				else if(buttonID == 8) ui.setActiveUI(MAP_UI);
				else if(buttonID == 9) ui.setActiveUI(SETTINGS_UI);
				else if(buttonID == 10) {
					if( ui.buttons[buttonID].isActive() ) tft.setBRT(LCD_DIMMED);
					else tft.setBRT(LCD_ON);
				}

				// BLUETOOTH
				else if(buttonID == 11) {
					if( ui.buttons[buttonID].isActive() ) ui.setBluetoothIcon(true);
					else ui.clearBluetoothIcon(true);
				}

				// TS DEBUG
				else if(buttonID == 12) {
					if( ui.buttons[buttonID].isActive() ) statesADESP.debugTouchInput = true;
					else statesADESP.debugTouchInput = false;
				}
				else;				
			}
		}		

		// Yield to other tasks
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}



//#######################################################################
// *TASK* >>> EVENT QUEUE RX
//#######################################################################
static void TASK_EVENTS(void *pvParameters) {
	
	LOG_INFO("EVENT TASK | EVENT HANDLER TASK STARTED");	
	
	for(;;) {
		if(xQueueEventRx != 0) {
			
			eventData_t event;
			xQueueReceive( xQueueEventRx, &(event), portMAX_DELAY);
			
			switch(event.type){

				//======================================================================	
				case ADSB_VECTOR_EVT: {
				//======================================================================
					LOG_INFO("EVENT TASK | AIRCRAFT VECTOR MSG");
					
					ui.addAircraftByVector(event.id.aircraft, event.data.vector, ADSB);
					break;
				}

				//======================================================================	
				case ADSB_LOCATION_EVT: {
				//======================================================================
					LOG_INFO("EVENT TASK | AIRCRAFT LOCATION MSG");
					
					ui.addAircraftByLocation(event.id.aircraft, event.data.location, ADSB);
					break;
				}
				

				//======================================================================				
				case REMOVE_STALE_AIRCRAFT: {
				//======================================================================
					//LOG_INFO("EVENT TASK | REMOVING STALE AIRCRAFTS");
					
					// Measure and draw battery
					ui.measureBattery();					
					ui.drawBattery(true);

					ui.removeStaleAircrafts();
					break;
				}


				case AIRPORT_LOCATION_EVT: {
					LOG_INFO("EVENT TASK | AIRPORT MSG")

					ui.addAirport(Airport::index++, 
								  Airport(event.id.airport, 
								          event.data.location));					
					break;
				}

				//======================================================================	
				default: {
				//======================================================================
					LOG_ERROR("EVENT TASK | UNKOWN EVENT");
					break;
				}
			}
		}
		
		// Yield to other tasks
		taskYIELD();
	}
}

//#######################################################################
// *Fx >>> ADESP SYSTEM INIT
//#######################################################################
void ADESP_PROCESS_FRAME(dataFrame_t xDTframe) {

	//####################################################
	// ADS-B DATA
	//####################################################

	// Frame Type: ADSB-B Location
	if(xDTframe.header.type == ADSB_LOCATION) {
		LOG_INFO("BT DATA | ADSB_LOCATION FRAME RX");

		eventData_t evt;
		evt.type = ADSB_LOCATION_EVT;
		evt.id.aircraft = xDTframe.header.id.aircraft;
		evt.data.location = xDTframe.location;
		
		xQueueSend( xQueueEventRx, (void *) &evt, 0 );
	}
	
	// Frame Type: ADS-B Vector
	if(xDTframe.header.type == ADSB_VECTOR) {
		LOG_INFO("BT DATA | ADSB_VECTOR FRAME RX");
		
		eventData_t evt;
		evt.type = ADSB_VECTOR_EVT;
		evt.id.aircraft = xDTframe.header.id.aircraft;
		evt.data.vector = xDTframe.vector;

		xQueueSend( xQueueEventRx, (void *) &evt, 0 );
	}

	//####################################################
	// MLAT DATA
	//####################################################

	// Frame Type: MLAT Location
	if(xDTframe.header.type == MLAT_LOCATION) {
		LOG_INFO("BT DATA | MLAT LOCATION FRAME RX");

		eventData_t evt;
		evt.type = MLAT_LOCATION_EVT;
		evt.id.aircraft = xDTframe.header.id.aircraft;
		evt.data.location = xDTframe.location;
		
		xQueueSend( xQueueEventRx, (void *) &evt, 0 );
	}
	
	// Frame Type: MLAT Vector
	if(xDTframe.header.type == MLAT_VECTOR) {
		LOG_INFO("BT DATA | MLAT VECTOR FRAME RX");
		
		eventData_t evt;
		evt.type = MLAT_VECTOR_EVT;
		evt.id.aircraft = xDTframe.header.id.aircraft;
		evt.data.vector = xDTframe.vector;

		xQueueSend( xQueueEventRx, (void *) &evt, 0 );
	}


	//####################################################
	// UAT DATA
	//####################################################

	// Frame Type: UAT Location
	if(xDTframe.header.type == UAT_LOCATION) {
		LOG_INFO("MBT DATA | UAT LOCATION FRAME RX");

		eventData_t evt;
		evt.type = TISB_LOCATION_EVT;
		evt.id.aircraft = xDTframe.header.id.aircraft;
		evt.data.location = xDTframe.location;;
		
		xQueueSend( xQueueEventRx, (void *) &evt, 0 );
	}
	
	// Frame Type: UAT Vector
	if(xDTframe.header.type == UAT_VECTOR) {
		LOG_INFO("BT DATA | UAT VECTOR FRAME RX");
		
		eventData_t evt;
		evt.type = TISB_VECTOR_EVT;
		evt.id.aircraft = xDTframe.header.id.aircraft;
		evt.data.vector = xDTframe.vector;

		xQueueSend( xQueueEventRx, (void *) &evt, 0 );
	}

	//####################################################
	// AIRPORT DATA
	//####################################################

	// Frame Type: Airport Location
	if(xDTframe.header.type == AIRPORT_LOCATION) {
		LOG_INFO("BT DATA | AIRPORT_LOCATION FRAME RX");

		eventData_t evt;
		evt.type = AIRPORT_LOCATION_EVT;
		strcpy(evt.id.airport, xDTframe.header.id.airport);		
		evt.id.airport[3] = 0;
		evt.data.location = xDTframe.location;
		
		xQueueSend( xQueueEventRx, (void *) &evt, 0 );
	}

	//####################################################
	// OWNSHIP DATA
	//####################################################
	
	// Frame Type: Local Position
	if(xDTframe.header.type == OWNSHIP_LOCATION) {
		LOG_INFO("BT DATA | OWNSHIP LOCATION FRAME RX");

		my.location = xDTframe.location;

		statesADESP.GPS = RECENT;
		statesADESP.lastGPSupdate = seconds();
	}
	
	// Frame Type: Local Vector
	if(xDTframe.header.type == OWNSHIP_VECTOR) {
		LOG_INFO("BT DATA | OWNSHIP VECTOR FRAME RX");

		my.vector = xDTframe.vector;
		ui.setCompass(-my.vector.heading, true);

		statesADESP.MAG = ACTIVE;
		statesADESP.lastMAGupdate = seconds();
	}
}


//#######################################################################
// *Fx >>> ADESP SYSTEM INIT
//#######################################################################
void ADESP_SYS_INIT(void) {

	// Startup Messages
	Serial.begin(ADESP_DEBUG_BAUD);	SYS_MSG();
	SYS_MSG("####################################################");
	SYS_MSG(">>                 ADESP-ETD 600                  <<");
	SYS_MSG("####################################################");	
	SYS_MSG("********  WARNING: USE AT YOUR OWN RISK!  **********");
	SYS_MSG("********      EXPERIMENTAL USE ONLY!      **********");
	SYS_MSG("####################################################");	
	SYS_MSGL(">> ADESP-ETD BUILD NUMBER :: ", ADESP_BUILD_NUM);
	SYS_MSGL(">> ADESP-ETD FIRMWARE VERSION :: ", ADESP_VER);
	SYS_MSG(">> INITIALIZING...");

	// Setup mutex and event queue
	xMutex = xSemaphoreCreateMutex();
	xQueueEventRx = xQueueCreate( QUEUE_LENGTH, sizeof(eventData_t) );

	xQueueTouchEvents = xQueueCreate( 20 , sizeof(eventData_t) );
	
	// Load Initial States
	statesADESP.display = ON;
	statesADESP.brightness = LCD_ON;
	statesADESP.currentRange = DEFAULT_RANGE;
	statesADESP.initComplete = false;
	statesADESP.tracking = 0;
	
	statesADESP.MAG = INACTIVE;
	statesADESP.GPS = INACTIVE;
	statesADESP.BLUETOOTH = INACTIVE;

	// Run Startup task
	xTaskCreate(TASK_INIT,
				"INIT", 
				INIT_TASK_STACK_SIZE, 
				NULL, 
				INIT_TASK_PRIORITY, 
				&xHandleInitTask);

	// Delay to acquire lock
	delay(100); 
}


//#######################################################################
// *Fx >>> ADESP SYSTEM START
//#######################################################################
void ADESP_SYS_START(void) {

	// Will Block until InitTask commpletes
	xSemaphoreTake( xMutex, portMAX_DELAY);

	// Start System Tasks (Event & Overlay)
	SYS_MSG(">> STARTING SYSTEM TASKS...");

	// Event handling task
	SYS_MSG2("   *EVENT SYSTEM TASK... ");
	xTaskCreatePinnedToCore(TASK_EVENTS,
							"EVENT_TSK",
							EVENT_TASK_STACK_SIZE,
							NULL,
							EVENT_TASK_PRIORITY, 
							&xHandleEventsTask, 0);
	SYS_MSG("OK");
	
	// Touch Event handling task
	SYS_MSG2("   *TOUCH SCREEN SYSTEM TASK... ");
	xTaskCreatePinnedToCore(TOUCH_EVENTS,
							"TOUCH_TSK",
							TOUCH_TASK_STACK_SIZE,
							NULL,
							TOUCH_TASK_PRIORITY, 
							&xHandleTouchTask, 0);
	SYS_MSG("OK");	
	
	// Overlay drawing task
	SYS_MSG2("   *SCREEN OVERLAY SYSTEM TASK... ");
	xTaskCreatePinnedToCore(TASKS_OVERLAY,
							"OVERLAY_TSK",
							OVERLAY_TASK_STACK_SIZE,
							NULL,
							OVERLAY_TASK_PRIORITY,
							&xHandleOverlayTask, 1);
	SYS_MSG("OK");

	// delay
	delay(500);

	// Initialize touch driver (try again if it fails the first time)
	if( ts.begin() );
	else if ( !ts.begin() ) tft.fillCircle(123, 14, 4, RED);
	else;


	
	// Release lock
	xSemaphoreGive( xMutex );	
	statesADESP.initComplete = true;	

	// Set initial UI
	ui.setActiveUI(MAIN_UI);

	SYS_MSG("|>>>>>>>>>>>>>>>>>> SYSTEM READY <<<<<<<<<<<<<<<<<<|");
}