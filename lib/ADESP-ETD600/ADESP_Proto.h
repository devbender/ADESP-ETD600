#include <ADESP_UI.h>

// MSG TYPES ----------------------------------------------------------
#define OWNSHIP_LOCATION  0xAA
#define OWNSHIP_VECTOR    0xAB

#define ADSB_LOCATION     0xBA
#define MLAT_LOCATION     0xBB
#define UAT_LOCATION     0xBC
#define AIRPORT_LOCATION  0xBD

#define ADSB_VECTOR   0xCA
#define MLAT_VECTOR   0xCB
#define UAT_VECTOR   0xCC

#define ADESP_FRAMESIZE 20

typedef struct {
  uint8_t type;   // 1 byte
  uint8_t seq;    // 1 byte
  uint8_t res1;   // 1 byte
  uint8_t res2;   // 1 byte
  union {         // 4 bytes
    uint32_t aircraft;
    char airport[4];
  } id;
} headerFrame_t;  // 8 bytes


typedef union {
  struct {
    headerFrame_t header;   // 8 bytes
    union {
      vector_t vector;      // 6 bytes
      location_t location;  // 12 bytes
    };
  };
  uint8_t buff[ADESP_FRAMESIZE];  // 20 bytes
} dataFrame_t;


enum event_t {
	ADSB_VECTOR_EVT,
  ADSB_LOCATION_EVT,
  MLAT_VECTOR_EVT,
  MLAT_LOCATION_EVT,
  TISB_VECTOR_EVT,
  TISB_LOCATION_EVT,
  AIRPORT_LOCATION_EVT,
	REMOVE_STALE_AIRCRAFT
};


struct eventData_t {
	event_t type;           // 4 bytes
  union {                 // 4 bytes
    uint32_t aircraft;
    char airport[4];
  } id;
	union dataFields {		  // 12 bytes
		point_t tsTouch;      // 4 bytes
		vector_t vector;      // 6 bytes
    location_t location;  // 12 bytes
	} data;		
};
