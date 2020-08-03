#include "ADESP_UI_Elements.h"

//#######################################################################
// GLOBALS
//#######################################################################
extern ADESP_HX8357 tft;


//#################################################################################################################################
// *CLASS* >>> UI BUTTONS CLASS
//#################################################################################################################################
ADESP_UI_Button::ADESP_UI_Button(int16_t _x, int16_t _y, uint16_t _w, uint16_t _h) {
    x = _x;
    y = _y;
    w = _w;
    h = _h;    
    region = {x, x+w, y, y+h}; 
}

ADESP_UI_Button::ADESP_UI_Button(const char* buttonLabel, int16_t _x, int16_t _y, uint16_t _w, uint16_t _h) {    
    strcpy(label, buttonLabel);
    x = _x;
    y = _y;
    w = _w;
    h = _h;
    region = {x, x+w, y, y+h}; 
}

void ADESP_UI_Button::init() {}


void ADESP_UI_Button::init(buttonType _type, uint8_t textSize) {    
    type = _type;
    labelTextSize = textSize;
}

void ADESP_UI_Button::setType(buttonType _type){
    type = _type;
}

void ADESP_UI_Button::setLabelTextSize(uint8_t _size) {
    labelTextSize = _size;
}

void ADESP_UI_Button::setImage(const char* _imgFile) {
     strcpy(imgFile, _imgFile);
}

void ADESP_UI_Button::setShaddowOffset(uint8_t _offset) {
    shaddowOffset = _offset;
}

void ADESP_UI_Button::draw() {

    if(type == ICON) {
        tft.fillRect(x+shaddowOffset, y+shaddowOffset, w, h, BLACK);    //  shaddow
        tft.drawFSJpeg(SPIFFS, imgFile, x, y);  // icon image
        tft.drawRect(x, y, w, h, outlineColor); // outline

        tft.setFont(&CalibriBold8pt7b);
		tft.setTextSize(labelTextSize); tft.setTextColor(textColor);		
		tft.printCentered(label, x+w/2, y+h+25);

        tft.setFont();
    }
    
    else if(type == SWITCH) {
        tft.fillRoundRect(x+shaddowOffset, y+shaddowOffset, w, h, h, BLACK);
        tft.fillRoundRect(x, y, w, h, h, GREY);

        tft.setFont(&CalibriBold8pt7b);
		tft.setTextSize(labelTextSize); tft.setTextColor(textColor);
        tft.setCursor(x-w - w/2, y+h/2 + 6);
		tft.print(label);

        tft.setFont();
    }

    else {

        // Draw button elements
        if(shaddowEnabled) // Shadow
            tft.fillRoundRect(x+shaddowOffset, y+shaddowOffset, w, h, 10, BLACK);
        if(fillColor) // Fill
            tft.fillRoundRect(x, y, w, h, 10, fillColor);
        // Outline
        tft.drawRoundRect(x, y, w, h, 10, outlineColor);
        
        // Draw toggle indicator
        if(type == TOGGLE) tft.fillCircle(x+8, y+8, 3, GREY);

        int16_t xT, yT;
        uint16_t wT, hT;

        tft.setTextSize(labelTextSize);
        tft.setTextColor(textColor);

        tft.getTextBounds(label, 0, 0, &xT, &yT, &wT, &hT);
        tft.setCursor(x + w/2 - wT/2, y + h/2 - hT/2);
        tft.print(label);
    }
}

void ADESP_UI_Button::drawPressed()  {
    if(type == TOGGLE) {
		//tft.fillCircle(x+8, y+8, 3, pressedColor);
        tft.drawRoundRect(x, y, w, h, 10, pressedColor);
    }
	else if(type == MOMENTARY) {
        tft.drawRoundRect(x, y, w, h, 10, pressedColor);
    }
    else if(type == ICON) {
        tft.drawRect(x, y, w, h, pressedColor);
    }
    else if(type == SWITCH) {
        //tft.drawRoundRect(x, y, w, h, h, pressedColor);
        //tft.fillRoundRect(x, y, w, h, h, GREY);
    }
}

void ADESP_UI_Button::drawReleased() {
	if(type == TOGGLE) {        
    	//tft.fillCircle(x+8, y+8, 3, GREY);
        tft.drawRoundRect(x, y, w, h, 10, outlineColor);
    }
	else if(type == MOMENTARY) {
		tft.drawRoundRect(x, y, w, h, 10, outlineColor);
    }
    else if(type == ICON) {
        tft.drawRect(x, y, w, h, outlineColor);
    }
    else if(type == SWITCH) {
        //tft.fillRoundRect(x, y, w, h, h, GREY);
        //tft.drawRoundRect(x, y, w, h, h, outlineColor);
    }
}


void ADESP_UI_Button::setFillColor(uint16_t _color) {
    fillColor = _color;
}

void ADESP_UI_Button::setOutlineColor(uint16_t _color) {
    outlineColor = _color;
}

void ADESP_UI_Button::setPressedColor(uint16_t _color) {
    pressedColor = _color;
}

void ADESP_UI_Button::setTextColor(uint16_t _color) {
    textColor = _color;
}

void ADESP_UI_Button::assignUI(ui_t newUI) {
	assignedUI = newUI;
}

ui_t ADESP_UI_Button::getAssignedUI() {
	return assignedUI;
}

void ADESP_UI_Button::setLabel(const char* buttonLabel) {
    strcpy(label, buttonLabel);    
}

void ADESP_UI_Button::setLabel(const char* buttonLabel, uint8_t textSize) {
    strcpy(label, buttonLabel);    
    labelTextSize = textSize;
}

bool ADESP_UI_Button::toggle() {
	if(toggled) toggled = false;
	else toggled = true;
	
	return toggled;
}

bool ADESP_UI_Button::isActive() {
	return toggled;
}

void ADESP_UI_Button::momentaryPress() {
    drawPressed();
    vTaskDelay(debounceDelay / portTICK_PERIOD_MS);
    drawReleased();
}

void ADESP_UI_Button::drawToggled(bool state) {
    if(type == TOGGLE) {
        if(state) tft.fillCircle(x+8, y+8, 3, pressedColor);
        else tft.fillCircle(x+8, y+8, 3, GREY);
    }
    else if(type == SWITCH) {
        if(state) {
            tft.fillRoundRect(x, y, w, h, h, GREY);
            tft.fillCircle(x+w - h/2-2, y + h/2, h/2-4, BLUE);
        }
        else {
            tft.fillRoundRect(x, y, w, h, h, GREY);
            tft.fillCircle(x + h/2+2, y + h/2, h/2-4, RGB(200, 200, 200) );
        }
    }
}

void ADESP_UI_Button::setActive() {
	toggled = true;	

    drawToggled(true);
    momentaryPress();
}

void ADESP_UI_Button::setInactive() {
	toggled = false;    
    momentaryPress();
    drawToggled(false);    
}

void ADESP_UI_Button::setDebounce(uint16_t delayMS) {
    debounceDelay = delayMS;
}

buttonType ADESP_UI_Button::getType() {
	return type;
}

screenRegion_t ADESP_UI_Button::getRegion() {
    return region;
}