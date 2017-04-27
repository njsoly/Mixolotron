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
#include "Mixolotron_RTC.h"

uint16_t get16BitColor(unsigned int r, unsigned int g, unsigned int b){
	r >>= 3;
	g >>= 2;
	b &= 0x1f;
	return ((r << 11) | (g << 5) | (b));
}
class Point {
	public:
	uint16_t x,y;
	Point(uint16_t x0=0, uint16_t y0=0){
		x = x0; 
		y = y0;
	}
	/** copy constructor? */
	Point(Point& p){
		Point(p.x, p.y);
	}
};
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
		//tft.drawString(String(*yyyymmdd).c_str(), x+1, y+2, 1, 0xFFFF);
	}
	void eraseFromLcd(Mixolotron_TFT& tft, unsigned int bg=0x000){
		tft.fillRectangle(p.x,p.y,8*length, 10, bg);
	}
	protected:
	Point p;
	unsigned int length = 6;
	unsigned int bg = 0x000;
};

class DateEntry {
	public:
	String yyyymmdd = String("      ");
	
	DateEntry(){
	}
	DateTime& getDateTime(){
		DateTime dt(year(), month(), day());
		return dt;
	}
	unsigned int year(){
		unsigned int y = 0;
		
		for(int i = 0; i < 4; i++){
			if(yyyymmdd[i] < '0' || yyyymmdd[i] > '9'){
				return -1;
			}
			y *= 10;
			y += (yyyymmdd[i] - '0');
		}
		return y;
		
	}
	unsigned int month(){
		unsigned int m = 0;
		for(uint8_t i = 0; i < 2; i++){
			if(yyyymmdd[i + 4] < '0' || yyyymmdd[i + 4] > '9'){
				return -1;
			}
			m *= 10;
			m += (yyyymmdd[i + 4] - '0');
		}
		return m;
	}
	unsigned int day(){
		uint8_t d = 0;
		for(uint8_t i = 5; i < 7; i++){
			if(yyyymmdd[i] < '0' || yyyymmdd[i] > '9'){
				return -1;
			}
			d *= 10;
			d += (yyyymmdd[i] - '0');
		}
		return d;
	}
	void paintToLcd(Mixolotron_TFT& tft){
		tft.fillRectangle(x,y,8*length, 10, 0x0222);
		tft.drawString(String(yyyymmdd).c_str(), x+1, y+2, 1, 0xFFFF);
	}
	void eraseFromLcd(Mixolotron_TFT& tft, unsigned int bg=0x000){
		tft.fillRectangle(x,y,8*length, 10, bg);
	}
	void clear(){
		yyyymmdd = "      ";
	}
	bool isClear(){
		for(int i = length - 1; i >= 0; i--){
			if(yyyymmdd[i] != ' '){
				return false;
			}
		}
		return true;
	}
	void backspace(){
		if(isClear() == true)	return;
		for(int i = length; i >= 0; i--){
			if(yyyymmdd[i] != ' '){
				yyyymmdd[i] = ' ';
				return;
			}
		}
	}
	void enterChar(char c){
		for(unsigned int i = 0; i < length; i++){
			if(yyyymmdd[i] == ' '){
				yyyymmdd[i] = c;
				return;
			}
		}
	}
	protected:
	unsigned int x=100, y=150, length = 6;	

	public:
	Point* getPosition(){
		return (new Point(x,y));
	}
	bool isFilled(){
		char c;
		for(unsigned int i = 0; i < length; i++){
			c = yyyymmdd[i];
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
enum STATE {INIT, CURRENT_DATE_ENTRY, DOB_ENTRY, WAITING_ON_PLC};
class Mixolotron {
	public:
	Mixolotron_RTC rtc;
	Mixolotron_keypad kp;
	Mixolotron_TFT tft;
	Mixolotron_serial serial;
	DateEntry dob;
	STATE state = INIT;
	Mixolotron(){
		
	};
	
	void loop(){
		int k = kp.getKey();
		if(k != -1)
			keypadInput(k);
		if(Serial.available() > 0){
			serialInput();
		}
	}
	
	void keypadInput(char c){
		switch(state){
			case DOB_ENTRY:
				if(c == 'B'){
					dob.backspace();
				}
				else if(c == 'C'){
					dob.clear();
				}
				else if(c == 'A'){
					if(dob.isFilled()){
						check21yoa();
					}
				}
				else dob.enterChar(c);
				dob.paintToLcd(tft);
				break;
			default: break;
		}
	}
	
	void serialInput(){
		
	}
	
	virtual void init(){
		serial.init();
		Wire.begin();
		rtc.begin();
		tft.init();
	}
	
	void enterDob(){
		state = DOB_ENTRY;
		dob.paintToLcd(tft);
		Point* p = dob.getPosition();
		tft.drawString("Enter DOB:",(p->x)-16,(p->y)-10,1,0x07FF);
		delete p;
	}
	
	
	protected:
	bool check21yoa(){
		
		DateTime then = dob.getDateTime();
		uint16_t yearDiff = currentDate->year() - then.year();
		if(yearDiff >= 22){
			return true;
		}
		else if (yearDiff < 21){
			return false;
		}
		uint16_t monthDiff = currentDate->month() - then.month();
		if(monthDiff > 0){
			return true;
		}
		else if (monthDiff < 0)
			return false;
		uint16_t dayDiff = currentDate->day() - then.day();
		if(dayDiff < 0)
			return false;
		return true;
	}
	private:
	DateTime* currentDate;
	
};




#endif