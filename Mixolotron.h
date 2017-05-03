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
//#include <new.h>
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

class ToastMessage {
	public:
	ToastMessage(char* message, unsigned int x, unsigned int y){
		msg = message;
		p = Point(x,y);
		p2 = Point(x+150, y+10);
	}
	void paintToLcd(Mixolotron_TFT& tft){
		tft.drawString(msg, p.x+1, p.y+1, 1, 0xffff);
		tft.drawRectangle(p.x,p.y,p2.x-p.x,p2.y-p.y,get16BitColor(0,255,255));
	}
	void eraseFromLcd(Mixolotron_TFT& tft, unsigned int bg=0x000){
		tft.fillRectangle(p.x, p.y, (p2.x-p.x)+2, (p2.y - p.y)+2, bg);
	}
	
	private:
	Point p, p2;
	char* msg;
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
		uint8_t _textsize = 2;
		tft.fillRectangle(x-37,y,8*length*_textsize,10*_textsize,0x000);
		tft.fillRectangle(x,y,8*length*_textsize, 10*_textsize, 0x0222);
		tft.drawString((*yyyymmdd), x+1, y+2, _textsize, 0xFFFF);
		tft.drawRectangle(x,y,8*length*_textsize, 10*_textsize, 0x7777);
	}
	void eraseFromLcd(Mixolotron_TFT& tft, unsigned int bg=0x000){
		uint8_t _textsize = 2;
		tft.fillRectangle(x-32,y-16,20+32+8*length*_textsize, 10+32, bg);
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
enum STATE {INIT, CURRENT_DATE_ENTRY, DOB_ENTRY, DOB_CHECK, NOT_21, SIGNAL_PLC, WAITING_ON_PLC_1, WAITING_ON_PLC_2};
class Mixolotron {
	public:
	Mixolotron_RTC rtc;
	Mixolotron_keypad kp;
	Mixolotron_TFT tft;
	DateEntry dob;
	STATE state = INIT;
	STATE lastState = INIT;
	Mixolotron(){
	};
	void printState(){
		String s = "state: ";
		if(state == INIT){
			s += "INIT";
		}
		else if(state == DOB_ENTRY){
			s += "DOB_ENTRY";
		}
		else if(state == DOB_CHECK){
			s += "DOB_CHECK";
		}
		else if(state == SIGNAL_PLC){
			s += "SIGNAL_PLC";
		}
		else if(state == NOT_21){
			s += "NOT_21";
		}
		else if(state == WAITING_ON_PLC_1){
			s += "WAITING_ON_PLC_1";
		}
		else if(state == WAITING_ON_PLC_2){
			s += "WAITING_ON_PLC_2";
		}
		else if(state == CURRENT_DATE_ENTRY){
			s += "CURRENT_DATE_ENTRY";
		}
		else {
			s += String(state);
		}
		
		Serial.println(s);
	}
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
		if(state == INIT){
			state = DOB_ENTRY;
		}
		else if(state == DOB_ENTRY){
			// print something?
		}
		else if(state == SIGNAL_PLC){
			signalPlcAndWait();
			return;
		}
		else if(state == WAITING_ON_PLC_1){
			
			if(state != lastState) Serial.println("waiting on plc #1");
			if (digitalRead(PLC_PIN_IN_1) != HIGH){
				Serial.println("pin in 1 is not HIGH");
			}
			else {
				// signal has changed, now see if it stays
				int debounceCt = 0;
				int debounceDelayMs = 10;
				while(digitalRead(PLC_PIN_IN_1) == HIGH){
					debounceCt++;
					delay(debounceDelayMs);
					Serial.println("debounce counter: " + String(debounceCt));
					if(debounceCt == 50){
							state = WAITING_ON_PLC_2;
						printChoiceMsg2();
						 break;
					}
				}
			}
		}
		else if(state == WAITING_ON_PLC_2){
			if(state != lastState) Serial.println("waiting on plc #2");
			pinMode(PLC_PIN_IN_SYSREADY, INPUT);
			if(digitalRead(PLC_PIN_IN_SYSREADY) != HIGH){
				Serial.println("pin in sysready is HIGH");
			}
			else {
				
				int debounceCt = 0;
				int debounceDelayMs = 10;
				Serial.println("debounce counter 2: " + String(debounceCt));
				while(digitalRead(PLC_PIN_IN_SYSREADY) == HIGH){

					debounceCt++;
					delay(debounceDelayMs);
					Serial.println("debounce counter 2: " + String(debounceCt));
					if(debounceCt == 50){
						state = DOB_ENTRY;
						//re-write dob entry screen
						enterDob();
						break;
					}
				}
			}
			
		}
		else if(state == NOT_21){
			userIsNot21();
		}
		
		lastState = state;
	}//end Mixolotron::loop()
	
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
	
	}//keypadInput()
	
	void serialInput(){
		
	}
	void acceptDob(){
		state = DOB_CHECK;
		Serial.println("dob entered: \"" + String(*dob.yyyymmdd) + "\"");
		bool is21 = check21yoa();
		if(is21){
			Serial.println("user is 21.");
			dob.clear();
			state = SIGNAL_PLC;
		}
		else {
			Serial.println("user is NOT 21.");
			dob.clear();
			state = NOT_21;
			//enterDob();
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
		printChoiceMsg1();
	}
	void printChoiceMsg1(){
		dob.eraseFromLcd(tft);
		ToastMessage pick1("choose alcohol type", 28, 150);
		pick1.paintToLcd(tft);
	}
	void printChoiceMsg2(){
		tft.fillRectangle(28, 150, 160, 20, 0x000);
		ToastMessage pick2("Choose mixer type",28,150);
		pick2.paintToLcd(tft);
	}
	
	void enterDob(){
		state = DOB_ENTRY;
		dob.paintToLcd(tft);
		Point* p = dob.getPosition();
		tft.drawString("Enter DOB (yyyymmdd):",(p->x)-8*4,(p->y)-10,1,0x07FF);
	}
	void userIsNot21(){
		Point p1(75,200);
		Point p2(200,200+10);
		Bounds b = Bounds(p1,p2);
		ToastMessage not21msg("user is not 21.",p1.x, p1.y);
		not21msg.paintToLcd(tft);
		delay(1500);
		not21msg.eraseFromLcd(tft);
		enterDob();
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
			//delete then;
			return true;
		}
		else if (yearDiff < 21){
			Serial.println("years bad: " + String(yearDiff));
			//delete then;
			return false;
		}
		uint16_t monthDiff = currentDate.month() - then->month();
		if(monthDiff > 0){
			//delete then;
			return true;
		}
		else if (monthDiff < 0){
			//delete then;
			return false;
		}
		uint16_t dayDiff = currentDate.day() - then->day();
		if(dayDiff >= 0){
			//delete then;
			return true;
			
		}
		else {
			//delete then;
			return false;
		}
	}
	private:
	DateTime currentDate;
	
};




#endif