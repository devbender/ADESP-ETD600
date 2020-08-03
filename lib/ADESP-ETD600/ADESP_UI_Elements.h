#pragma once

#include <ADESP_UI_Types.h>

typedef void(*callback_t)();

//########################################################################################################
// UI BUTTON CLASS
//########################################################################################################
class ADESP_UI_Button {

public:
    uint16_t id;
    char label[16];
    uint8_t labelTextSize = 1;   

    // Dimensions
    uint16_t x,y;
    uint16_t w,h;
    
    // Colors
    uint16_t fillColor = 0;
    uint16_t outlineColor = WHITE;    
    uint16_t pressedColor = GREEN;
    uint16_t textColor = WHITE;

    uint8_t shaddowOffset = 4;
    
    buttonType type = MOMENTARY;
    screenRegion_t region;

    uint16_t debounceDelay = 200;

    bool shaddowEnabled = true;
    bool toggled = false;
	
    ui_t assignedUI;

    callback_t callback = nullptr;

    char imgFile[32];
    const GFXfont* font;

public:
    ADESP_UI_Button() {}
    ADESP_UI_Button(int16_t _x, int16_t _y, uint16_t _w, uint16_t _h);
    ADESP_UI_Button(const char* buttonLabel, int16_t _x, int16_t _y, uint16_t _w, uint16_t _h);
    ADESP_UI_Button(const char* buttonLabel, int16_t _x, int16_t _y, uint16_t _w, uint16_t _h, uint16_t _color);

public:    
    void init();	
    void init(buttonType); // button type
    void init(buttonType, uint8_t); // button type, text size
    
    void setCallback(callback_t);

    void draw();
    void drawPressed();
    void drawReleased();
    void drawToggled(bool);

	ui_t getAssignedUI();
	void assignUI(ui_t);

    void setFillColor(uint16_t _color);
    void setOutlineColor(uint16_t _color);
    void setPressedColor(uint16_t _color);
    void setTextColor(uint16_t _color);
    void setShaddowOffset(uint8_t _offset);
	
	void setType(buttonType);
    void setLabelTextSize(uint8_t _size);
    void setLabel(const char*); // label text
    void setLabel(const char*, uint8_t); // label text, text size    
    
    bool toggle();
	void momentaryPress();   
   
    bool isActive();
    void setActive();
    void setInactive();

    void setDebounce(uint16_t); // debounce ms
    
    buttonType getType();
    screenRegion_t getRegion();

    void setImage(const char* _imgFile);
    void setFont(const GFXfont* _font);
};