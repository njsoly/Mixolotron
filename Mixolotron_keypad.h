/*** Mixolotron_keypad.h ***/
#ifndef MIXOLOTRON_KEYPAD_H_
#define MIXOLOTRON_KEYPAD_H_

#include <Arduino.h>

#ifndef ROW_PINS
#define ROW_PINS 42,43,44,45
#endif
#ifndef COLUMN_PINS
#define COLUMN_PINS 46,47,48,49
#endif
	
unsigned char rows[]{ROW_PINS};
unsigned char cols[]{COLUMN_PINS};


int getKey(){

	int row = -1, column = -1;
	for(int checkCol = 0; checkCol < 4; checkCol++){
		for(int i = 0; i < 4; i++){
			pinMode(cols[i], OUTPUT);
			digitalWrite(cols[i], LOW);
			pinMode(rows[i], OUTPUT);
			digitalWrite(rows[i], LOW);
			delay(2);
		}
		for(int checkRow = 0; checkRow < 4; checkRow++){
			pinMode(rows[checkRow], OUTPUT);
			pinMode(cols[checkCol], INPUT);

			digitalWrite(rows[checkRow], HIGH);
			delay(2);
			if(digitalRead(cols[checkCol]) == HIGH ){
				column = checkCol;
				row = checkRow; 
				checkRow = 4;
				checkCol = 4;
				
			}
			pinMode(cols[checkCol], OUTPUT);
			digitalWrite(rows[checkRow], LOW);
			delay(1);
		}
	}
	

	if(row != -1 && column != -1){
		Serial.println(String(F("pressed: row=")) + String(row) + String(F(", col=")) + String(column));
		return (row ) * 4 + (column + 0);
	}
	else {
		Serial.println(String(F("nothing pressed")));
		return -1;
	}
}



#endif // MIXOLOTRON_KEYPAD_H_