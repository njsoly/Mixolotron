/**** Mixolotron_serial.h ****/
#ifndef MIXOLOTRON_SERIAL_H_
#define MIXOLOTRON_SERIAL_H_

#ifndef SERIAL_BAUD 
#define SERIAL_BAUD 57600
#endif

class Mixolotron_serial {
public:
	
	Mixolotron_serial(unsigned int baud=SERIAL_BAUD){
		this->baudRate = baud;
	}
	
protected:
	unsigned int baudRate;
	
};



#endif // MIXOLOTRON_SERIAL_H_