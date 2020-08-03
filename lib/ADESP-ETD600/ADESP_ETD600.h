#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "Arduino.h"

#include "FS.h"
#include "SD.h"
#include "SPIFFS.h"

#include <EEPROM.h>
#include <BluetoothSerial.h>

#include <unordered_map>

#include <ADESP_Log.h>
#include <ADESP_Proto.h>
#include <ADESP_HX8357.h>
#include <ADESP_STMPE610.h>

#include <ADESP_UI.h>


//#####################################################################
// SETTINGS 
//#####################################################################
#define OVERLAY_DRAW_DELAY 1000

#define LOG_LEVEL_WARNING
#define ADESP_DEBUG_BAUD 115200

#define LCD_DIM_TIME_SECONDS 300
#define LCD_OFF_TIME_SECONDS 360

// TASK DEFINES -------------------------------------------------------
#define EVENT_TASK_STACK_SIZE	1024*8
#define TOUCH_TASK_STACK_SIZE	1024*8

#define OVERLAY_TASK_STACK_SIZE	1024*8
#define INIT_TASK_STACK_SIZE	1024*8
#define BATTERY_TASK_STACK_SIZE	1024*2

#define INIT_TASK_PRIORITY	    (configMAX_PRIORITIES - 1)  // HIGH
#define TOUCH_TASK_PRIORITY	    (configMAX_PRIORITIES - 1)  // HIGH
#define EVENT_TASK_PRIORITY	    (configMAX_PRIORITIES - 5)  // MID-HIGH
#define OVERLAY_TASK_PRIORITY	(tskIDLE_PRIORITY + 5)		// LOW

#define QUEUE_LENGTH 50

// EEPROM ADDRESSES ----------------------------------------------------
#define EEPROM_SIZE 1
#define EEPROM_ADDR_RANGE 0

//#####################################################################################################################
// VARIABLE AND TYPE DEFINITIONS
//#####################################################################################################################
void IRAM_ATTR onTS_TOUCH();
void IRAM_ATTR onTS_TOUCH2();
void ADESP_PROCESS_FRAME(dataFrame_t);
void ADESP_SYS_INIT(void);
void ADESP_SYS_START(void);

template <typename T> 
int ADESP_SAVE_SETTINGS(int address, const T& settings) {

    const uint8_t* p = (const uint8_t*)(const void*)&settings;
    unsigned int i;
    
	for (i = 0; i < sizeof(settings); i++)
    	EEPROM.write(address++, *p++);
    
	EEPROM.commit();

	return i;
}

template <typename T> 
int ADESP_LOAD_SETTINGS(int address, T& settings) {

    uint8_t* p = (uint8_t*)(void*)&settings;
    unsigned int i;
    
	for (i = 0; i < sizeof(settings); i++)
    	*p++ = EEPROM.read(address++);
    
	return i;
}

/*
void listDir(fs::FS &fs, const char * dirname, uint8_t levels=0){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
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
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}
*/