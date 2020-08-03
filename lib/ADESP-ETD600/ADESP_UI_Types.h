#pragma once

#include <unordered_map>
#include <ADESP_HX8357.h>
#include <ADESP_STMPE610.h>
#include <ADESP_Font.h>

// LCD 16 BIT COLORS --------------------------------------------------
#define BLACK		0x0000
#define WHITE		0xFFFF	
#define BLUE		0x001F
#define RED			0xF800
#define GREEN		0x07E0
#define GREEN2		0x0523
#define CYAN		0x07FF
#define MAGENTA		0xF81F
#define YELLOW		0xFFE0
#define YELLOW2		0xFEA0
#define YELLOW3		0xCEA0
#define YELLOW4		0xFEE0
#define GREY		0x8410
#define DGREY		0x528A
#define DGREY2		0x2104
#define SKYBLUE		0x041F
#define GNDBROWN	0x8208
#define ORANGE      0xFC42

// RGB to 565 coversion
static inline uint16_t RGB(uint8_t R, uint8_t G, uint8_t B) {
  return ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3);
}

//#####################################################################1
// UI COLOR DEFINITIONS
//#####################################################################
#define UI_BACKGROUND_COLOR 		RGB( 60, 60, 60 )
#define UI_TOP_BAR_COLOR			RGB( 0, 0, 0 )
#define UI_TOP_BAR_DIV_LINE_COLOR	RGB( 150, 150, 150 )
#define UI_OUTER_CIRCLE_COLOR		RGB( 150, 150, 150 )
#define UI_INNER_CIRCLE_COLOR		RGB( 255, 255, 255 )
#define UI_MY_AIRCRAFT_COLOR		RGB( 255, 0, 0 )

#define UI_BUTTON_FILL_COLOR 		RGB( 70, 70, 70 )

#define UI_INNER_CIRCLE_DEG			15

// SYSTEM  -----------------------------------------------------------
#define SYSTEM_NAME "ADESP-ETD600"
#define ADESP_VER "v0.1"
#define ADESP_BUILD_NUM "000000"

// BATTERY VOLTAGE RANGE ----------------------------------------------
#define BATT_MAX 4500
#define BATT_SAMPLES 10
#define BATT_SAMPLES_DELAY 10
#define BATT_MEASUREMENT_RATE_SECONDS 15
#define BATT_MIN (BATT_MAX * 0.8) // Lipo 80% rule

// UI DEFINES 
#define AIRCRAFT_SIZE	6
#define VECTOR_SIZE 	20
#define AIRPORT_SIZE	3
#define AIRCRAFT_TTL 	30

#define MIN_RANGE 10
#define MAX_RANGE 250
#define DEFAULT_RANGE 40

// EARTH RADIUS ------------------------------------------------------
//Metric - Metric (m, km, km/h)  Imperial (ft, NM, knots)
#define EARTH_RAD_KM	6371
#define EARTH_RAD_NM	3440

struct screenRegion_t {
	int xL, xR;
	int yT, yB;
};

enum buttonType {
    TOGGLE,
    MOMENTARY,
	ICON,
	SWITCH
};

struct point_t {
	uint16_t x, y;
};

struct tftPoint_t {
	uint16_t x, y;
};

enum states_t { 
	OLD, RECENT,
	ACTIVE, INACTIVE,
	ON, OFF, DIMMED
};

struct program_states_t {
	states_t MAG;
	states_t GPS;
    states_t BLUETOOTH;
    states_t display;
	
	// States
	uint8_t battery;
	uint8_t brightness;
	uint16_t currentRange;
	uint16_t tracking;	
    
	// Timing
	uint16_t lastGPSupdate;
	uint16_t lastMAGupdate;
	uint16_t lastTStouch;

	char time[16];

	// Flags
    bool northUp = true;
	bool initComplete = false;
	bool drawTraffic = true;
	bool drawAirports = false;
	bool debugTouchInput = false;
};

struct settings_t {

};

struct location_t {
	int alt;
	float lat, lon;
};

struct vector_t {
	uint16_t speed, heading;
	int16_t vrate;
};

enum traffic_t {
  ADSB,
  MLAT,
  TISB
};

enum ui_t {
	NULL_UI,
    MAIN_UI,
    TRAFFIC_UI,
    WEATHER_UI,
	MAP_UI,
	SETTINGS_UI,
	COMMON
};

struct mydata_t {
	vector_t vector{0,0,0};
	location_t location{0, 0.00, 0.00};
};

inline bool operator ==(const point_t &ts, const screenRegion_t &reg) {
	return	ts.x > reg.xL && ts.x < reg.xR &&
		ts.y > reg.yT && ts.y < reg.yB;
};

static inline bool operator !=(const location_t &loc1, const location_t &loc2) {
	return 	loc1.lat != loc2.lat ||
		loc1.lon != loc2.lon;
};

static inline bool operator !=(const vector_t &vect1, const vector_t &vect2) {
	return 	vect1.speed != vect2.speed ||
		vect1.heading != vect2.heading ||
		vect1.vrate != vect2.vrate;
};