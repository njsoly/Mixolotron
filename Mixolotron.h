#ifndef MIXOLOTRON_H_
#define MIXOLOTRON_H_

//***         Mixolotron.h         ***//


#include <Arduino.h>
#include <stdint.h>
#include "Mixolotron_EEPROM.h"
#include "Mixolotron_TFT.h"
#include "Mixolotron_keypad.h"
#include "Mixolotron_serial.h"
#include <Wire.h>
#include <new.h>
#include "Mixolotron_RTC.h"
#include <stdlib.h>
#include "Point.h"
#include "Bounds.h"

uint16_t get16BitColor(unsigned int r, unsigned int g, unsigned int b){
	r >>= 3;
	g >>= 2;
	b &= 0x1f;
	return ((r << 11) | (g << 5) | (b));
}
class TFT_GUI {
	public:
	TFT_GUI(unsigned int x=100, unsigned int y=150){
		p = Point(x,y);
	}
	TFT_GUI(Point& point){
		p = Point(point);
	}
	virtual void paintToLcd(Mixolotron_TFT& tft){
		tft.fillRectangle(p.x,p.y,8*length, 10, 0x0222);
		//tft.drawString(String((*yyyymmdd)).c_str(), x+1, y+2, 1, 0xFFFF);
	}
	void eraseFromLcd(Mixolotron_TFT& tft, unsigned int bg=0x000){
		tft.fillRectangle(p.x,p.y,8*length, 10, bg);
	}
	protected:
	Point p;
	unsigned int length = 6;
	unsigned int bg = 0x000;
};
class Date {
public:
	Date(uint16_t year, uint8_t m, uint8_t d){
		yyyy=year;
		mm = m;
		dd = d;
	}
	uint16_t yyyy;
	uint8_t mm, dd;
	uint16_t year(){ return yyyy; }
	uint8_t month(){ return mm; }
	uint8_t day(){ return dd; }
};
class DateEntry {
	public:
	char * yyyymmdd[24];
	
	DateEntry(){
	(*yyyymmdd)= "        ";
	}
	Date* getDateTime(){
		Date* dt = new Date(year(), month(), day());
		return dt;
	}
	unsigned int year(){
		unsigned int y = 0;
		
		for(int i = 0; i < 4; i++){
			if((*yyyymmdd)[i] < '0' || (*yyyymmdd)[i] > '9'){
				return -1;
			}
			y *= 10;
			y += ((*yyyymmdd)[i] - '0');
		}
		Serial.println("year of " + String(*yyyymmdd) + " is " + String(y));
		return y;
		
	}
	unsigned int month(){
		unsigned int m = 0;
		for(uint8_t i = 0; i < 2; i++){
			if((*yyyymmdd)[i + 4] < '0' || (*yyyymmdd)[i + 4] > '9'){
				return -1;
			}
			m *= 10;
			m += ((*yyyymmdd)[i + 4] - '0');
		}
		return m;
	}
	unsigned int day(){
		uint8_t d = 0;
		for(uint8_t i = 5; i < 7; i++){
			if((*yyyymmdd)[i] < '0' || (*yyyymmdd)[i] > '9'){
				return -1;
			}
			d *= 10;
			d += ((*yyyymmdd)[i] - '0');
		}
		return d;
	}
	void paintToLcd(Mixolotron_TFT& tft){
		uint8_t _sz = 2;
		tft.fillRectangle(x,y,8*length*_sz, 10*_sz, 0x0222);
		tft.drawString((*yyyymmdd), x+1, y+2, _sz, 0xFFFF);
	}
	void eraseFromLcd(Mixolotron_TFT& tft, unsigned int bg=0x000){
		tft.fillRectangle(x,y,8*length, 10, bg);
	}
	void clear(){
		int i = 0; 
		while ((*yyyymmdd)[i]){
			(*yyyymmdd)[i] = ' ';
			i++;
		}
	}
	bool isClear(){
		for(int i = length - 1; i >= 0; i--){
			if((*yyyymmdd)[i] != ' '){
				return false;
			}
		}
		return true;
	}
	void backspace(){
		if(isClear() == true)	return;
		for(int i = length-1; i >= 0; i--){
			if((*yyyymmdd)[i] != ' '){
				(*yyyymmdd)[i] = (char) ' ';
				return;
			}
		}
	}
	void enterChar(int c){
		for(unsigned int i = 0; i < length; i++){
			if(((*yyyymmdd))[i] == ' '){
				((*yyyymmdd))[i] = (char) c;
				return;
			}
		}
	}
	protected:
	unsigned int x=64, y=150, length = 8;	
	
	public:
	Point* getPosition(){
		return (new Point(x,y));
	}
	bool isFilled(){
		char c;
		for(unsigned int i = 0; i < length; i++){
			c = ((*yyyymmdd))[i];
			if(c >= '0' && c <= '9'){
				
			}
			else if(c == ' '){
				return false;
			}
			else {
				return false;
			}
		}
		return true;
	}

};
enum STATE {INIT, CURRENT_DATE_ENTRY, DOB_ENTRY, DOB_CHECK, SIGNAL_PLC, WAITING_ON_PLC_1, WAITING_ON_PLC_2};
class Mixolotron {
	public:
	Mixolotron_RTC rtc;
	Mixolotron_keypad kp;
	Mixolotron_TFT tft;
	Mixolotron_serial serial;
	DateEntry dob;
	STATE state = INIT;
	STATE lastState = INIT;
	Mixolotron(){
	};
	void updateDateTime(DateTime& dt){
		currentDate = dt;
	}
	void updateDateTime(){
		currentDate = rtc.now();	
	}
	void loop(){
		int k = kp.getKeyChar();
		if(k != -1){
			keypadInput(k);
			Serial.println("key pressed: " + String((char) k));
		}
		if(Serial.available() > 0){
			serialInput();
		}
		if(state == DOB_ENTRY){
			// print something?
		}
		if(state == SIGNAL_PLC){
			signalPlcAndWait();
		}
		else if(state == WAITING_ON_PLC_1){
			
			if(state != lastState) Serial.println("waiting on plc #1");
			if(digitalRead(PLC_PIN_IN_1) == HIGH){
				// print "choose mixer" on display
				state = WAITING_ON_PLC_2;
			}
			
		}
		else if(state == WAITING_ON_PLC_2){
			if(state != lastState) Serial.println("waiting on plc #2");
			if(digitalRead(PLC_PIN_IN_SYSREADY == HIGH)){
				//re-write dob entry screen
				enterDob();
				
			}
		}
		lastState = state;
	}
	
	void keypadInput(char c){
		if(c == 'B'){
			dob.backspace();
		}
		else if(c == 'C'){
			dob.clear();
		}
		else if(c == 'A'){
			if(dob.isFilled()){
				acceptDob();
			}
			else {
					Serial.println("enter full date yyyymmdd.");
			}
		}
		else dob.enterChar(c);
		
		dob.paintToLcd(tft);
	
	}
	
	void serialInput(){
		
	}
	void acceptDob(){
		state = DOB_CHECK;
		Serial.println("dob entered: \"" + String(*dob.yyyymmdd) + "\"");
		bool is21 = check21yoa();
		if(is21){
			Serial.println("user is 21.");
			state = SIGNAL_PLC;
		}
		else {
			Serial.println("user is NOT 21.");
			dob.clear();
			enterDob();
		}
		
	}
	virtual void init(){
		Serial.begin(57600);
		Wire.begin();
		rtc.begin();
		tft.init();
		currentDate = rtc.now();
		pinMode(PLC_PIN_OUT, OUTPUT);
		pinMode(PLC_PIN_IN_1, INPUT);
		pinMode(PLC_PIN_IN_SYSREADY, INPUT);
	}
	
	void signalPlcAndWait(){
		pinMode(PLC_PIN_OUT, OUTPUT);
		digitalWrite(PLC_PIN_OUT, HIGH);
		state = WAITING_ON_PLC_1;
		
	}
	
	void enterDob(){
		state = DOB_ENTRY;
		dob.paintToLcd(tft);
		Point* p = dob.getPosition();
		tft.drawString("Enter DOB (yyyymmdd):",(p->x)-8*4,(p->y)-10,1,0x07FF);
		delete p;
	}
	
	
	protected:
	bool check21yoa(){
		Serial.println("<check21yoa()>");
		Date* then = dob.getDateTime();
		int yearThen, yearNow;
		yearNow = currentDate.year();
		yearThen = then->yyyy;
		Serial.println("yearThen: " + String(yearThen));
		Serial.println("yearNow: " + String(yearNow));
		if(yearThen > 3000) yearThen -= 2000;
		int yearDiff = yearNow - yearThen;
		if(yearDiff >= 22){
			Serial.println("years check out: " + String(yearDiff));
			delete then;
			return true;
		}
		else if (yearDiff < 21){
			Serial.println("years bad: " + String(yearDiff));
			delete then;
			return false;
		}
		uint16_t monthDiff = currentDate.month() - then->month();
		if(monthDiff > 0){
			delete then;
			return true;
		}
		else if (monthDiff < 0){
			delete then;
			return false;
		}
		uint16_t dayDiff = currentDate.day() - then->day();
		if(dayDiff >= 0){
			delete then;
			return true;
			
		}
		else {
			delete then;
			return false;
		}
	}
	private:
	DateTime currentDate;
	
};




#endif