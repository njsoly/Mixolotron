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
class Mixolotron_keypad {
	public:
	unsigned int lastPress = 0;
	unsigned int minimumPressInterval = 300;
	
	int timeSinceLastPress(){
		return millis() - lastPress;
	}
	
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
			if(millis() - lastPress >= minimumPressInterval){
			#if (defined(DEBUGALLSERIAL) || defined(DEBUG)) && !defined(NOSERIAL)
			Serial.println(String(F("pressed: row=")) + String(row) + String(F(", col=")) + String(column));
			#endif
				lastPress = millis();
				return (row ) * 4 + (column + 0);
			}
			else { // new keypress is too soon, return -1 as if no button pressed.
				return -1;
			}
		}
		else {
			#if (defined(DEBUGALLSERIAL) || defined(DEBUG)) && !defined(NOSERIAL)
			Serial.println(String(F("nothing pressed")));
			#endif
			return -1;
		}
	}

	int getKeyChar(){
		int keyCode = getKey();
		if(keyCode == -1){
			return -1;
		}
		else if (keyCode >= 0 && keyCode <= 15){
			switch(keyCode){
				case 0:
				return '1'; break;
				case 1:
				return '2'; break;
				case 2: return '3'; break;
				case 3: return 'A'; break;
				case 4: return '4'; break;
				case 5: return '5'; break;
				case 6: return '6'; break;
				case 7: return 'B'; break;
				case 8: return '7'; break;
				case 9: return '8'; break;
				case 10: return '9'; break;
				case 11: return 'C'; break;
				case 12: return '*'; break;
				case 13: return '0'; break;
				case 14: return '#'; break;
				case 15: return 'D'; break;
				default: return -1;
			}
		}
		else return -1;
	}
}; // class Mixolotron_keypad

#endif // MIXOLOTRON_KEYPAD_H_